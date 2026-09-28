"""Exercise real Effect loader mailbox failures without Client, D3D, or assets."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from Tools.ValtanPipeline.test_valtan_sequence_append_native import ROOT, toolchain

PROBE = r'''#include "Effect_LoadPreparationJob.cpp"
#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
using namespace Client;
using namespace std::chrono_literals;
int checks = 0;
void check(bool condition) {
    ++checks;
    if (!condition) throw std::runtime_error("check " + std::to_string(checks));
}
EFFECT_LOAD_FAILURE_RECEIPT failure(uint64_t epoch=7, uint64_t revision=4) {
    return {epoch, revision, "effect.failed.target", -2147467259LL, "exact renderer commit rejection"};
}
EFFECT_LOAD_JOB_RESULT staged() {
    EFFECT_LOAD_JOB_RESULT result;
    result.eKind = EFFECT_LOAD_JOB_RESULT_KIND::TARGET_STAGED;
    result.iJobEpoch=7; result.iCatalogRevision=4;
    result.strEffectAssetId="effect.pending";
    result.pImmutablePayload=std::make_shared<const int>(1);
    return result;
}
int main() {
    CEffectLoadPreparationJob job;
    std::string status;
    std::optional<EFFECT_LOAD_JOB_COMMAND> displaced;
    check(!job.Record_FirstFailure(failure()));
    check(job.Open(7,4,status));
    auto invalid=failure(); invalid.iRootCode=0;
    check(!job.Record_FirstFailure(invalid));
    check(!job.Record_FirstFailure(failure(6,4)));
    check(!job.Record_FirstFailure(failure(7,3)));
    check(job.Push_Result_Wait(staged())==EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED);
    check(job.Push_Result_Wait(staged())==EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED);
    std::promise<void> started;
    auto blocked=std::async(std::launch::async,[&] {
        started.set_value(); return job.Push_Result_Wait(staged());
    });
    started.get_future().wait();
    check(job.Record_FirstFailure(failure()));
    auto later=failure(); later.strRootMessage="later cancellation noise";
    check(!job.Record_FirstFailure(later));
    auto copy=job.Get_FirstFailure(); copy->strRootMessage="changed copy";
    check(job.Get_FirstFailure()->strRootMessage==failure().strRootMessage);
    check(job.Post_Command(EFFECT_LOAD_JOB_COMMAND::Cancel(7,4),displaced,status)==EFFECT_LOAD_MAILBOX_POST_RESULT::POSTED);
    check(blocked.wait_for(2s)==std::future_status::ready);
    check(blocked.get()==EFFECT_LOAD_RESULT_PUSH_RESULT::CANCELLED);
    check(job.Is_Cancelled() && job.Get_PendingResultCount()==0);
    auto saved=job.Get_FirstFailure();
    check(saved && saved->iJobEpoch==7 && saved->iCatalogRevision==4 &&
        saved->strEffectAssetId==failure().strEffectAssetId &&
        saved->iRootCode==failure().iRootCode && saved->strRootMessage==failure().strRootMessage);
    check(!job.Record_FirstFailure(later));
    EFFECT_LOAD_PROGRESS_SNAPSHOT progress; progress.iJobEpoch=7; progress.iCatalogRevision=4;
    check(!job.Publish_Progress(progress));
    check(job.Open(8,4,status) && !job.Get_FirstFailure());
    check(job.Record_FirstFailure(failure(8,4)));
    check(job.Post_Command(EFFECT_LOAD_JOB_COMMAND::Rebase(9,5,{"effect.new"},std::make_shared<const int>(2)),displaced,status)==EFFECT_LOAD_MAILBOX_POST_RESULT::POSTED);
    check(!job.Get_FirstFailure());
    check(!job.Record_FirstFailure(failure(8,4)));
    check(job.Record_FirstFailure(failure(9,5)));
    check(job.Post_Command(EFFECT_LOAD_JOB_COMMAND::Close(9,5),displaced,status)==EFFECT_LOAD_MAILBOX_POST_RESULT::REPLACED);
    check(job.Is_Closed() && job.Get_FirstFailure()->iJobEpoch==9);
    check(job.Post_Command(EFFECT_LOAD_JOB_COMMAND::Cancel(9,5),displaced,status)==EFFECT_LOAD_MAILBOX_POST_RESULT::CLOSED);
    check(job.Get_FirstFailure()->strRootMessage==failure().strRootMessage);
    check(job.Open(10,5,status) && !job.Get_FirstFailure());
    check(job.Post_Command(EFFECT_LOAD_JOB_COMMAND::Cancel(10,5),displaced,status)==EFFECT_LOAD_MAILBOX_POST_RESULT::POSTED);
    check(!job.Get_FirstFailure());
    std::cout<<checks<<" checks PASS\n";
}
'''


class TestEffectLoadFailureReceipt(unittest.TestCase):
    def test_first_cause_survives_cancel_and_close_and_resets_with_epoch(self):
        with tempfile.TemporaryDirectory(prefix="EffectLoadFailure-") as directory:
            out=Path(directory)
            cl, environment=toolchain(out)
            source=out/"probe.cpp"
            source.write_text(PROBE,encoding="utf-8")
            binary=out/"probe.exe"
            compiled=subprocess.run([str(cl),"/nologo","/EHsc","/std:c++20","/MDd",
                "/I"+str(ROOT/"Client/Public"),"/I"+str(ROOT/"Client/Private"),str(source),
                "/Fo"+str(out/"probe.obj"),"/Fe"+str(binary),"/Fd"+str(out/"probe.pdb")],
                env=environment,cwd=out,capture_output=True)
            self.assertEqual(0,compiled.returncode,(compiled.stdout+compiled.stderr).decode(errors="replace"))
            ran=subprocess.run([str(binary)],capture_output=True,timeout=15)
            self.assertEqual(0,ran.returncode,(ran.stdout+ran.stderr).decode(errors="replace"))
            self.assertIn(b"30 checks PASS",ran.stdout)
            print(ran.stdout.decode().strip())


if __name__=="__main__":
    unittest.main()
