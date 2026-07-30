# Git 멀티세션 PR·병합 가이드

> 목적: 여러 Codex 세션과 팀원이 동시에 작업할 때 서로의 변경을 섞지 않고, 충돌을 PR 전에 해결해 `main`에 안전하게 병합한다.

## 1. 먼저 기억할 세 문장

1. `git push`는 원격 브랜치만 갱신하며 PR을 자동 생성하지 않는다.
2. 한 번 병합된 브랜치에 새 커밋을 계속 넣지 말고, 최신 `origin/main`에서 새 브랜치를 만든다.
3. 세션마다 별도 branch와 별도 worktree를 사용하면 다른 세션의 미커밋 파일을 건드리지 않는다.

충돌을 무조건 없애는 명령은 없다. 아래 절차의 목적은 충돌을 merge 버튼을 누른 뒤가 아니라 작업자 브랜치에서 미리 발견하고 해결하는 것이다.

## 2. 세션 하나당 branch·worktree 하나

기존 작업 폴더가 더럽거나 다른 세션이 사용 중이어도 branch를 바꾸거나 stash하지 않는다. 새 세션은 저장소 옆에 독립 worktree를 만든다.

아래 `my-topic`을 실제 작업명으로 바꾼다.

```bash
cd /c/Users/user/Desktop/LostArk
git fetch origin
git worktree add ../LostArk-my-topic -b feature/my-topic origin/main
cd ../LostArk-my-topic
```

완료 확인:

```bash
git branch --show-current
git status -sb
```

기대 결과:

```text
feature/my-topic
## feature/my-topic
```

규칙:

- 같은 branch를 두 세션이 동시에 사용하지 않는다.
- 같은 물리 worktree를 두 세션이 동시에 수정하지 않는다.
- 세션 시작 메시지에 `branch`, `worktree`, 담당 파일을 기록한다.
- 다른 세션의 미커밋 변경을 `stash`, `clean`, `reset`, `checkout --`로 치우지 않는다.
- 의존 작업이 있으면 해당 PR이 `main`에 병합된 뒤 새 branch를 만든다.

## 3. 커밋에는 요청 파일만 넣는다

전체 추가 명령을 습관적으로 사용하지 않는다.

```bash
# 금지에 가까운 사용
git add .
git add -A

# 권장
git add -- "에셋 추출 파이프라인.md"
git add -- Client/Private/MapTool.cpp Client/Public/MapTool.h
```

커밋 전 확인:

```bash
git diff --cached --name-status
git diff --cached --check
git diff --cached
```

스테이징 목록에 요청과 무관한 파일이 하나라도 있으면 커밋하지 않는다.

```bash
git restore --staged -- "잘못 추가한 파일"
```

검증 후 커밋:

```bash
git commit -m "docs: add extraction troubleshooting guide"
```

## 4. PR 전에 최신 main과 충돌을 끝낸다

세션이 단독 소유한 branch라면 PR 직전에 rebase한다.

```bash
git fetch origin
git rebase origin/main
```

충돌이 없으면 바로 검증한다. 충돌이 있으면:

```bash
git status
rg -n "^(<<<<<<<|=======|>>>>>>>)"
```

충돌 파일을 직접 읽고 두 변경의 의도를 합친 뒤, 해결한 파일만 추가한다.

```bash
git add -- "충돌을 해결한 파일"
git rebase --continue
```

잘못 해결했으면 억지로 진행하지 않는다.

```bash
git rebase --abort
```

주의:

- `--ours`, `--theirs`를 폴더 전체에 일괄 적용하지 않는다.
- conflict marker만 지우고 빌드가 된다고 완료하지 않는다. 양쪽 기능이 모두 남았는지 확인한다.
- 이미 다른 사람이 같이 쓰는 원격 branch는 rebase하지 않는다. 그 경우 `git merge origin/main`으로 history를 보존한다.
- rebase한 branch를 이전에 push했고 본인만 소유한다면 일반 force가 아니라 `--force-with-lease`만 사용한다.

## 5. 검증 후 push한다

문서 전용 변경:

```bash
git diff --check origin/main...HEAD
git diff --name-status origin/main...HEAD
```

C++·Engine·Client 변경은 저장소 규칙대로 검증한다.

```text
1. Engine x64 Debug/Release
2. UpdateLib.bat Debug/Release
3. Client x64 Debug/Release
```

검증이 끝난 branch를 push한다.

```bash
git push -u origin HEAD
```

rebase 후 본인만 사용하는 기존 원격 branch를 갱신할 때만:

```bash
git push --force-with-lease origin HEAD
```

## 6. push 뒤 PR을 직접 생성한다

GitHub CLI 연결 확인:

```bash
gh auth status
```

PR 생성:

```bash
gh pr create \
  --repo tnestyle70/LostArk \
  --base main \
  --head feature/my-topic \
  --draft \
  --fill
```

`--head`는 현재 branch 이름으로 바꾼다. PR 본문에는 최소한 다음을 적는다.

```text
변경 내용
변경 이유와 원인
사용자·팀 영향
실행한 검증
실행하지 못한 검증과 이유
```

push 후 PR이 자동으로 생길 것이라고 기다리지 않는다. 생성 여부를 확인한다.

```bash
gh pr status
gh pr view --web
```

## 7. merge 가능한 상태를 확인한다

리뷰 요청 전:

```bash
gh pr view --json number,url,isDraft,mergeable,mergeStateStatus
gh pr checks --watch
```

완료 조건:

- PR changed files가 요청 범위와 정확히 일치한다.
- `mergeable`이 `MERGEABLE`이다.
- required check가 전부 통과한다.
- review 반영 후 unresolved thread가 없다.
- 최신 `main`과 다시 비교해 conflict가 없다.

Draft를 리뷰 가능 상태로 바꾼다.

```bash
gh pr ready
```

## 8. merge와 후처리

팀의 현재 history처럼 merge commit을 유지하려면:

```bash
gh pr merge --merge --delete-branch
```

병합 후 기존 기본 worktree에서 `main`을 갱신한다. 그 worktree에 다른 세션의 미커밋 변경이 있으면 branch 전환부터 하지 말고 해당 세션이 끝날 때까지 기다린다.

```bash
git fetch origin
git switch main
git pull --ff-only origin main
```

독립 worktree의 작업이 끝났고 그 안에 미커밋 파일이 없는지 확인한 뒤 제거한다.

```bash
cd /c/Users/user/Desktop/LostArk
git worktree list
git worktree remove ../LostArk-my-topic
git worktree prune
```

## 9. 이미 더러운 공유 작업 폴더를 만났을 때

다음 상태에서는 pull, rebase, branch switch를 실행하지 않는다.

```text
다른 세션이 수정한 파일이 git status에 보임
누가 만든 파일인지 모름
같은 파일을 다른 세션도 편집 중임
```

처리 순서:

1. 기존 변경은 그대로 둔다.
2. `git fetch origin`만 실행한다.
3. `origin/main`에서 새 worktree와 새 branch를 만든다.
4. 새 worktree에서 작업·검증·commit·push·PR을 완료한다.
5. 기존 작업 폴더를 정리하거나 되돌리지 않는다.

## 10. 복사해서 쓰는 최종 체크리스트

```text
[ ] 세션 전용 worktree와 feature branch다.
[ ] 최신 origin/main에서 시작했다.
[ ] 다른 세션과 담당 파일이 겹치지 않는다.
[ ] 정확한 경로만 git add 했다.
[ ] cached diff가 요청 범위와 일치한다.
[ ] main rebase/merge 후 conflict가 없다.
[ ] 문서 검사 또는 Debug/Release 빌드를 완료했다.
[ ] git push -u origin HEAD를 실행했다.
[ ] gh pr create로 PR을 실제 생성했다.
[ ] changed files, checks, mergeable 상태를 확인했다.
[ ] merge 후 새 작업에는 기존 branch를 재사용하지 않는다.
```
