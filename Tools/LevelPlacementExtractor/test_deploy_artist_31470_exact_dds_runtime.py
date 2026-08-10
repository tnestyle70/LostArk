from __future__ import annotations

import copy
import json
import os
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock

import artist_31470_exact_dds_runtime_deployment_approval as approval
import deploy_artist_31470_exact_dds_runtime as deployer


class ExactDdsRuntimeDeploymentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.contract = deployer.load_contract()
        candidates = []
        configured = os.environ.get("ARTIST_31470_EXACT_DDS_SOURCE_ROOT")
        if configured:
            candidates.append(Path(configured))
        candidates.extend(
            [
                deployer.ROOT / ".codex_tmp/artist-f-dds-recovery/source-export",
                Path(
                    "C:/Users/user/Desktop/Resource_LostArk/"
                    "01_Extracted/Effect/WARLORD/CorePackages"
                ),
                Path(
                    "C:/Users/user/Desktop/Resource_LostArk/"
                    "01_Extracted/Effect/VALTAN/ActionBound"
                ),
            ]
        )
        cls.provider_root = next(
            (
                candidate
                for candidate in candidates
                if candidate.is_dir()
                and all(
                    (
                        candidate
                        / Path(
                            proposal["sourceExactDdsEvidence"][
                                "sourceExtractedDdsRelativePath"
                            ]
                        )
                    ).is_file()
                    for proposal in cls.contract.proposals
                )
            ),
            None,
        )
        if cls.provider_root is None:
            raise RuntimeError(
                "Artist 31470 exact DDS provider is required; set "
                "ARTIST_31470_EXACT_DDS_SOURCE_ROOT"
            )
        deployer.inspect_source_files(cls.contract, cls.provider_root)

    def setUp(self) -> None:
        self.temp_directory = tempfile.TemporaryDirectory()
        self.root = Path(self.temp_directory.name)
        self.source_root = self.root / "Source"
        self.runtime_root = self.root / "Client/Bin/Resources"
        self.output = self.root / "Data/deployment.receipt.json"
        self.runtime_root.parent.mkdir(parents=True)
        self.output.parent.mkdir(parents=True)
        for proposal in self.contract.proposals:
            relative = proposal["sourceExactDdsEvidence"][
                "sourceExtractedDdsRelativePath"
            ]
            source = self.provider_root / Path(relative)
            target = self.source_root / Path(relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)

    def tearDown(self) -> None:
        self.temp_directory.cleanup()

    def deploy(self, *, require_approval: bool = True) -> dict:
        return deployer.deploy_transaction(
            self.contract,
            self.source_root,
            self.runtime_root,
            self.output,
            require_approval=require_approval,
        )

    def target_paths(self) -> list[Path]:
        return [
            self.runtime_root / Path(proposal["proposedRuntimeAssetId"])
            for proposal in self.contract.proposals
        ]

    def populate_exact_targets(self) -> None:
        for proposal in self.contract.proposals:
            source = self.source_root / Path(
                proposal["sourceExactDdsEvidence"]["sourceExtractedDdsRelativePath"]
            )
            target = self.runtime_root / Path(proposal["proposedRuntimeAssetId"])
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)

    def assert_all_targets_absent(self) -> None:
        self.assertTrue(all(not target.exists() for target in self.target_paths()))
        self.assertFalse(self.output.exists())

    def assert_all_targets_exact(self) -> None:
        for index, (target, proposal) in enumerate(
            zip(self.target_paths(), self.contract.proposals, strict=True)
        ):
            deployer.verify_dds_file(
                target,
                proposal["sourceExactDdsEvidence"],
                f"test target {index}",
            )

    def assert_no_transaction_residue(self) -> None:
        residues = list(
            self.runtime_root.parent.glob(deployer.TRANSACTION_DIRECTORY_PREFIX + "*")
        )
        self.assertEqual([], residues)

    def assert_no_recovery_backup(self) -> None:
        self.assertFalse(
            (self.runtime_root.parent / deployer.RECOVERY_DIRECTORY_NAME).exists()
        )

    def patch_casefold_sibling(self, directory: Path, sibling_name: str):
        original = Path.iterdir
        expected = deployer.windows_path_identity(directory)

        def iter_with_collision(path):
            entries = list(original(path))
            if deployer.windows_path_identity(Path(path)) == expected:
                entries.append(Path(path) / sibling_name)
            return iter(entries)

        return mock.patch.object(
            Path,
            "iterdir",
            autospec=True,
            side_effect=iter_with_collision,
        )

    def test_absent_targets_deploy_all_four(self) -> None:
        receipt = self.deploy()
        deployer.validate_receipt(self.contract, receipt, require_approval=False)
        self.assert_all_targets_exact()
        self.assertEqual(
            ["ABSENT"] * 4,
            [row["targetBefore"]["status"] for row in receipt["assets"]],
        )
        self.assertFalse(receipt["admission"]["sourceExactMaterialClaim"])
        self.assertFalse(receipt["admission"]["r4Complete"])
        self.assertFalse(receipt["admission"]["productReady"])
        deployer.verify_recovery_backup(self.contract, receipt, self.runtime_root)
        self.assertEqual(0, receipt["recoveryBackup"]["backupPayloadFileCount"])
        self.assertEqual(4, receipt["recoveryBackup"]["absentTargetMarkerCount"])
        self.assert_no_transaction_residue()

    def test_existing_exact_targets_are_backed_up_and_admitted(self) -> None:
        self.populate_exact_targets()
        receipt = self.deploy()
        self.assert_all_targets_exact()
        self.assertEqual(
            ["PRESENT_EXACT_EQUAL"] * 4,
            [row["targetBefore"]["status"] for row in receipt["assets"]],
        )
        deployer.verify_recovery_backup(self.contract, receipt, self.runtime_root)
        self.assertEqual(4, receipt["recoveryBackup"]["backupPayloadFileCount"])
        self.assertEqual(0, receipt["recoveryBackup"]["absentTargetMarkerCount"])

    def test_existing_mismatched_target_fails_before_transaction(self) -> None:
        target = self.target_paths()[0]
        target.parent.mkdir(parents=True)
        target.write_bytes(b"different team-managed payload")
        before = target.read_bytes()
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assertEqual(before, target.read_bytes())
        self.assertTrue(all(not path.exists() for path in self.target_paths()[1:]))
        self.assertFalse(self.output.exists())

    def test_deploy_is_idempotent_when_receipt_and_targets_exist(self) -> None:
        first = self.deploy()
        first_bytes = self.output.read_bytes()
        second = self.deploy()
        self.assertEqual(first, second)
        self.assertEqual(first_bytes, self.output.read_bytes())
        self.assert_all_targets_exact()

    def test_approval_projection_normalizes_allowed_prestate(self) -> None:
        absent = self.deploy()
        absent_projection = approval.receipt_projection_sha256(absent)
        shutil.rmtree(self.output.parent)
        self.output.parent.mkdir()
        exact = deployer.build_receipt(
            self.contract,
            tuple(
                deployer.TargetBefore(
                    "PRESENT_EXACT_EQUAL",
                    proposal["sourceExactDdsEvidence"]["dds"]["byteCount"],
                    proposal["sourceExactDdsEvidence"]["dds"]["rawSha256"],
                )
                for proposal in self.contract.proposals
            ),
        )
        self.assertEqual(absent_projection, approval.receipt_projection_sha256(exact))

    def test_source_path_wrong_case_is_rejected(self) -> None:
        original = self.source_root / "FX_TEX_00"
        temporary = self.source_root / "renamed"
        wrong_case = self.source_root / "fx_tex_00"
        original.rename(temporary)
        temporary.rename(wrong_case)
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assert_all_targets_absent()

    def test_source_casefold_sibling_collision_is_rejected(self) -> None:
        with self.patch_casefold_sibling(self.source_root, "fx_tex_00"):
            with self.assertRaises(deployer.ContractError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_runtime_path_wrong_case_is_rejected(self) -> None:
        self.runtime_root.mkdir()
        (self.runtime_root / "effect").mkdir()
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assert_all_targets_absent()

    def test_runtime_casefold_sibling_collision_is_rejected(self) -> None:
        self.runtime_root.mkdir()
        (self.runtime_root / "Effect").mkdir()
        with self.patch_casefold_sibling(self.runtime_root, "effect"):
            with self.assertRaises(deployer.ContractError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_recovery_backup_casefold_sibling_collision_rolls_back(self) -> None:
        recovery_parent = self.runtime_root.parent / deployer.RECOVERY_DIRECTORY_NAME
        recovery_parent.mkdir()
        with self.patch_casefold_sibling(
            self.runtime_root.parent,
            deployer.RECOVERY_DIRECTORY_NAME.casefold(),
        ):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assertTrue(recovery_parent.is_dir())
        self.assertEqual([], list(recovery_parent.iterdir()))
        self.assert_no_transaction_residue()

    def test_output_equal_to_runtime_target_fails_before_any_write(self) -> None:
        self.output = self.target_paths()[0]
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assertFalse(self.runtime_root.exists())
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_output_casefold_equal_to_runtime_target_fails_before_any_write(self) -> None:
        target = self.target_paths()[0]
        self.output = target.with_name(target.name.upper())
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assertFalse(self.runtime_root.exists())
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_output_equal_to_recovery_manifest_fails_before_any_write(self) -> None:
        receipt = deployer.build_receipt(
            self.contract,
            tuple(
                deployer.TargetBefore("ABSENT", None, None)
                for _ in self.contract.proposals
            ),
        )
        self.output = self.runtime_root.parent.joinpath(
            *Path(receipt["recoveryBackup"]["manifestRelativePath"]).parts
        )
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assertFalse(self.runtime_root.exists())
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_output_inside_transaction_stage_namespace_fails_before_any_write(self) -> None:
        self.output = (
            self.runtime_root.parent
            / f"{deployer.TRANSACTION_DIRECTORY_PREFIX}attack"
            / "stage/0.dds"
        )
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assertFalse(self.runtime_root.exists())
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_source_symlink_or_reparse_point_is_rejected(self) -> None:
        source = self.source_root / Path(
            self.contract.proposals[0]["sourceExactDdsEvidence"][
                "sourceExtractedDdsRelativePath"
            ]
        )
        real = source.with_name(source.name + ".real")
        source.rename(real)
        try:
            source.symlink_to(real)
        except OSError:
            source.write_bytes(real.read_bytes())
            original = deployer._has_reparse_point
            with mock.patch.object(
                deployer,
                "_has_reparse_point",
                side_effect=lambda path: path == source or original(path),
            ):
                with self.assertRaises(deployer.ContractError):
                    self.deploy()
        else:
            with self.assertRaises(deployer.ContractError):
                self.deploy()
        self.assert_all_targets_absent()

    def test_runtime_symlink_or_reparse_parent_is_rejected(self) -> None:
        self.runtime_root.mkdir()
        external = self.root / "external-effect"
        external.mkdir()
        effect = self.runtime_root / "Effect"
        try:
            effect.symlink_to(external, target_is_directory=True)
        except OSError:
            effect.mkdir()
            original = deployer._has_reparse_point
            with mock.patch.object(
                deployer,
                "_has_reparse_point",
                side_effect=lambda path: path == effect or original(path),
            ):
                with self.assertRaises(deployer.ContractError):
                    self.deploy()
        else:
            with self.assertRaises(deployer.ContractError):
                self.deploy()
        self.assertFalse(self.output.exists())

    def test_corrupt_source_bytes_fail_before_transaction(self) -> None:
        source = self.source_root / Path(
            self.contract.proposals[2]["sourceExactDdsEvidence"][
                "sourceExtractedDdsRelativePath"
            ]
        )
        payload = bytearray(source.read_bytes())
        payload[-1] ^= 0xFF
        source.write_bytes(payload)
        with self.assertRaises(deployer.ContractError):
            self.deploy()
        self.assert_all_targets_absent()

    def test_partial_stage_copy_failure_leaves_all_targets_absent(self) -> None:
        original = shutil.copyfile
        stage_copy_count = 0

        def fail_second_stage_copy(source, target, *args, **kwargs):
            nonlocal stage_copy_count
            if Path(target).parent.name == "stage":
                stage_copy_count += 1
                if stage_copy_count == 2:
                    raise OSError("injected partial stage copy failure")
            return original(source, target, *args, **kwargs)

        with mock.patch.object(deployer.shutil, "copyfile", side_effect=fail_second_stage_copy):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_backup_copy_failure_preserves_existing_exact_targets(self) -> None:
        self.populate_exact_targets()
        original = shutil.copyfile
        backup_copy_count = 0

        def fail_second_backup_copy(source, target, *args, **kwargs):
            nonlocal backup_copy_count
            if Path(target).parent.name == "backup":
                backup_copy_count += 1
                if backup_copy_count == 2:
                    raise OSError("injected backup copy failure")
            return original(source, target, *args, **kwargs)

        with mock.patch.object(deployer.shutil, "copyfile", side_effect=fail_second_backup_copy):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_exact()
        self.assertFalse(self.output.exists())
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_second_asset_move_failure_rolls_back_all_four(self) -> None:
        original = os.replace
        commit_move_count = 0

        def fail_second_commit_move(source, target):
            nonlocal commit_move_count
            if Path(source).parent.name == "stage":
                commit_move_count += 1
                if commit_move_count == 2:
                    raise OSError("injected second asset move failure")
            return original(source, target)

        with mock.patch.object(deployer.os, "replace", side_effect=fail_second_commit_move):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_move_failure_restores_existing_exact_targets(self) -> None:
        self.populate_exact_targets()
        before = [target.read_bytes() for target in self.target_paths()]
        original = os.replace
        commit_move_count = 0

        def fail_third_commit_move(source, target):
            nonlocal commit_move_count
            if Path(source).parent.name == "stage":
                commit_move_count += 1
                if commit_move_count == 3:
                    raise OSError("injected third asset move failure")
            return original(source, target)

        with mock.patch.object(deployer.os, "replace", side_effect=fail_third_commit_move):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assertEqual(before, [target.read_bytes() for target in self.target_paths()])
        self.assert_all_targets_exact()
        self.assertFalse(self.output.exists())
        self.assert_no_recovery_backup()

    def test_post_verify_failure_rolls_back_all_four(self) -> None:
        with mock.patch.object(
            deployer,
            "_post_verify_targets",
            side_effect=deployer.ContractError("injected post-verify failure"),
        ):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_receipt_move_failure_rolls_back_all_four(self) -> None:
        original = os.replace
        failed = False

        def fail_receipt_move(source, target):
            nonlocal failed
            if Path(target) == self.output and not failed:
                failed = True
                raise OSError("injected receipt move failure")
            return original(source, target)

        with mock.patch.object(deployer.os, "replace", side_effect=fail_receipt_move):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_postreceipt_target_mutation_rolls_back_all_four(self) -> None:
        original = os.replace
        receipt_committed = False

        def mutate_after_receipt_commit(source, target):
            nonlocal receipt_committed
            result = original(source, target)
            if Path(target) == self.output and not receipt_committed:
                receipt_committed = True
                self.target_paths()[0].write_bytes(b"injected postreceipt mutation")
            return result

        with mock.patch.object(
            deployer.os,
            "replace",
            side_effect=mutate_after_receipt_commit,
        ):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_postreceipt_recovery_mutation_rolls_back_all_four(self) -> None:
        original = os.replace
        receipt_committed = False

        def mutate_recovery_after_receipt_commit(source, target):
            nonlocal receipt_committed
            result = original(source, target)
            if Path(target) == self.output and not receipt_committed:
                receipt_committed = True
                receipt = deployer.read_strict_json(self.output)
                backup_directory = deployer.recovery_backup_directory(
                    receipt,
                    self.runtime_root,
                )
                (backup_directory / "rollback-manifest.json").write_bytes(
                    b"injected postreceipt recovery mutation"
                )
            return result

        with mock.patch.object(
            deployer.os,
            "replace",
            side_effect=mutate_recovery_after_receipt_commit,
        ):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assert_no_transaction_residue()

    def test_legacy_manifest_identity_receipt_refresh_writes_no_runtime_asset(self) -> None:
        receipt = self.deploy()
        target_bytes = [target.read_bytes() for target in self.target_paths()]
        backup_directory = deployer.recovery_backup_directory(receipt, self.runtime_root)
        manifest_path = backup_directory / "rollback-manifest.json"
        manifest_bytes = manifest_path.read_bytes()

        legacy = copy.deepcopy(receipt)
        backup = legacy["recoveryBackup"]
        legacy["recoveryBackup"] = {
            "anchor": backup["anchor"],
            "relativeDirectory": backup["relativeDirectory"],
            "manifestRelativePath": backup["manifestRelativePath"],
            "manifestSha256": backup["manifestCanonicalSelfSha256"],
            "backupPayloadFileCount": backup["backupPayloadFileCount"],
            "absentTargetMarkerCount": backup["absentTargetMarkerCount"],
            "preservedAfterCommit": backup["preservedAfterCommit"],
        }
        legacy.pop("receiptSha256")
        legacy["receiptSha256"] = deployer.canonical_sha256(legacy)
        self.output.write_bytes(deployer.serialized_json_bytes(legacy))

        refreshed = deployer.refresh_deployment_receipt(
            self.contract,
            self.source_root,
            self.runtime_root,
            self.output,
        )
        self.assertEqual(target_bytes, [target.read_bytes() for target in self.target_paths()])
        self.assertEqual(manifest_bytes, manifest_path.read_bytes())
        self.assertEqual(
            deployer.raw_sha256(manifest_path),
            refreshed["recoveryBackup"]["manifestRawSha256"],
        )
        self.assertEqual(
            deployer.read_strict_json(manifest_path)["manifestSha256"],
            refreshed["recoveryBackup"]["manifestCanonicalSelfSha256"],
        )

    def test_receipt_runtime_asset_swap_reseal_is_rejected_without_approval(self) -> None:
        receipt = self.deploy()
        mutated = copy.deepcopy(receipt)
        mutated["assets"][0]["runtimeAssetId"] = mutated["assets"][1]["runtimeAssetId"]
        mutated["assets"][0]["rowSha256"] = deployer.canonical_sha256(
            deployer.row_without_digest(mutated["assets"][0])
        )
        mutated.pop("receiptSha256")
        mutated["receiptSha256"] = deployer.canonical_sha256(mutated)
        with self.assertRaises(deployer.ContractError):
            deployer.validate_receipt(
                self.contract,
                mutated,
                require_approval=False,
            )

    def test_receipt_unknown_key_reseal_is_rejected_without_approval(self) -> None:
        receipt = self.deploy()
        mutated = copy.deepcopy(receipt)
        mutated["unexpected"] = True
        mutated.pop("receiptSha256")
        mutated["receiptSha256"] = deployer.canonical_sha256(mutated)
        with self.assertRaises(deployer.ContractError):
            deployer.validate_receipt(
                self.contract,
                mutated,
                require_approval=False,
            )

    def test_receipt_authority_identity_reseal_is_rejected_without_approval(self) -> None:
        receipt = self.deploy()
        mutated = copy.deepcopy(receipt)
        mutated["sourceEvidence"]["authorityCommit"] = "0" * 40
        mutated.pop("receiptSha256")
        mutated["receiptSha256"] = deployer.canonical_sha256(mutated)
        with self.assertRaises(deployer.ContractError):
            deployer.validate_receipt(
                self.contract,
                mutated,
                require_approval=False,
            )

    def test_receipt_source_serial_reseal_is_rejected_without_approval(self) -> None:
        receipt = self.deploy()
        mutated = copy.deepcopy(receipt)
        row = mutated["assets"][0]
        row["sourceExactDdsEvidence"]["sourceTexture2D"]["serialSha256"] = "0" * 64
        row["rowSha256"] = deployer.canonical_sha256(deployer.row_without_digest(row))
        mutated.pop("receiptSha256")
        mutated["receiptSha256"] = deployer.canonical_sha256(mutated)
        with self.assertRaises(deployer.ContractError):
            deployer.validate_receipt(
                self.contract,
                mutated,
                require_approval=False,
            )

    def test_strict_json_rejects_bom_and_duplicate_keys(self) -> None:
        bom = self.root / "bom.json"
        bom.write_bytes(b"\xef\xbb\xbf{}")
        duplicate = self.root / "duplicate.json"
        duplicate.write_text('{"schema":1,"schema":2}', encoding="utf-8")
        with self.assertRaises(deployer.ContractError):
            deployer.read_strict_json(bom)
        with self.assertRaises(deployer.ContractError):
            deployer.read_strict_json(duplicate)

    def test_partial_directory_creation_failure_removes_created_directories(self) -> None:
        original = Path.mkdir
        created_count = 0

        def fail_third_runtime_directory(path, *args, **kwargs):
            nonlocal created_count
            path = Path(path)
            if path == self.runtime_root or self.runtime_root in path.parents:
                created_count += 1
                if created_count == 3:
                    raise OSError("injected directory creation failure")
            return original(path, *args, **kwargs)

        with mock.patch.object(
            Path,
            "mkdir",
            autospec=True,
            side_effect=fail_third_runtime_directory,
        ):
            with self.assertRaises(deployer.DeploymentError):
                self.deploy()
        self.assert_all_targets_absent()
        self.assert_no_recovery_backup()
        self.assertFalse(self.runtime_root.exists())
        self.assert_no_transaction_residue()

    def test_binding_coordinated_reseal_is_rejected_by_frozen_bytes(self) -> None:
        binding = copy.deepcopy(self.contract.binding_receipt)
        binding["provisioningProposals"][0]["proposedRuntimeAssetId"] = (
            "Effect/Artist/Textures/FX_TEX_00/forged.dds"
        )
        binding["provisioningProposals"][0]["rowSha256"] = deployer.canonical_sha256(
            deployer.row_without_digest(binding["provisioningProposals"][0])
        )
        binding.pop("receiptSha256")
        binding["receiptSha256"] = deployer.canonical_sha256(binding)
        mutated_binding = self.root / "mutated-binding.json"
        mutated_binding.write_text(
            json.dumps(binding, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        with self.assertRaises(deployer.ContractError):
            deployer.load_contract(mutated_binding, deployer.DEFAULT_EXACT_DDS_RECEIPT)


if __name__ == "__main__":
    unittest.main()
