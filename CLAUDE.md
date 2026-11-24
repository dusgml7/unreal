# CLAUDE.md — 축영(逐影) 언리얼 프로젝트 작업 규칙

이 파일은 Claude Code가 이 저장소에서 작업할 때 지켜야 할 규칙이다. 작업 전에 반드시 아래 문서를 먼저 읽는다.

- `docs/game-design.md` — 확정된 기획 내용 (게임 규칙, 수치, 아이템, 은신처)
- `docs/tasks.md` — 지금 구현할 작업 목록과 완료 조건

## 프로젝트 기본 정보

- 엔진: **Unreal Engine 5.7**
- 열어야 하는 프로젝트 파일: **`game.uproject`** (C++ 모듈 이름: `game`)
  - `BellTest2`, `CharacterTest`, `dollbujeokdeadmouse.uproject`는 같은 Content를 쓰는 테스트용 파일이다. 수정하지 않는다.
- C++ 코드 위치: `Source/game/`
- 팀원이 만든 블루프린트/에셋: `Content/Content/` (`BPI_Interact`, `BP_Collectible`, `bujeok`, `deadMouse`, `strawDoll`, `Character` 등)
- 나머지 `Content/` 하위 폴더(GeunjeongjeonComplex, Kwalnaegaksa 등)는 구매한 에셋 팩이다. 수정하지 않는다.
- Git LFS 사용 중 (`.uasset`, `.umap`, `.png`, `.fbx`, `.wav`, `.jpg`).

## 가장 중요한 규칙

1. **`.uasset`, `.umap` 파일은 직접 만들거나 고치지 않는다.** 바이너리 파일이라 텍스트로 수정할 수 없고, 깨지면 팀 전체 프로젝트가 망가진다.
2. 기능은 **C++ 클래스로 만들고, 블루프린트에서 상속·연결해서 쓰도록** 설계한다.
   - 수치는 전부 `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="...")`로 노출해서 기획자가 에디터에서 바꿀 수 있게 한다.
   - 이벤트는 `BlueprintImplementableEvent` 또는 `BlueprintAssignable` 델리게이트로 노출해서 사운드·이펙트·UI 연결을 블루프린트에서 할 수 있게 한다.
   - UI는 C++ `UUserWidget` 부모 클래스를 만들고 `meta=(BindWidget)` / `BindWidgetOptional`로 위젯을 연결한다. 실제 위젯 블루프린트(WBP_)와 디자인은 사람이 에디터에서 만든다.
3. 기획서에 수치나 동작이 정해지지 않은 부분은 **임의로 확정하지 말고** 기본값을 넣은 뒤 코드 주석에 `// TODO(기획 확인):`을 남기고, 작업 끝에 확인이 필요한 목록을 정리해서 보고한다.
4. 기존 팀원 블루프린트(`BPI_Interact`, `BP_Collectible`)와 충돌하지 않게 한다. C++ 쪽에 상호작용 인터페이스가 필요하면 새 이름(`UHorrorInteractable` 등)으로 만들고, 기존 블루프린트 인터페이스와 어떻게 이어 붙일지 보고한다.
5. `Config/*.ini` 수정이 필요하면 바꾸기 전에 무엇을 왜 바꾸는지 먼저 설명한다. 특히 `DefaultEngine.ini`의 맵 설정은 건드리지 않는다.
6. 작업은 `main`이 아닌 **새 브랜치**에서 한다 (예: `feature/gauge-system`). 커밋과 푸시는 사용자가 요청할 때만 한다.

## 코드 스타일

- 클래스 접두사는 언리얼 규칙을 따른다 (`U`, `A`, `F`, `E`, `I`).
- 새 클래스 이름 앞에 프로젝트 접두사 `Chuk`을 붙인다. 예: `UChukGaugeComponent`, `AChukHideSpot`.
- 주석은 **한국어**로 쓴다. 팀원 대부분이 C++에 익숙하지 않으므로, 블루프린트에서 쓰는 방법을 헤더 주석에 적는다.
- 하드코딩된 에셋 경로(`ConstructorHelpers`)는 쓰지 않는다. 사운드·위젯 클래스 등은 `UPROPERTY`로 받아서 에디터에서 지정한다.

## 빌드 방법 (Windows)

에디터를 **닫은 상태**에서 아래 명령으로 빌드한다.

```
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" gameEditor Win64 Development -Project="C:\Users\user\Documents\GitHub\unreal\game.uproject" -WaitMutex
```

- 엔진 경로가 다르면 사용자에게 물어본다.
- 새 모듈 의존성(UMG 등)이 필요하면 `Source/game/game.Build.cs`에 추가한다. UI 작업에는 `"UMG"`, `"Slate"`, `"SlateCore"`가 필요하다.
- 새 C++ 클래스를 추가한 뒤에는 Live Coding이 아니라 위 명령으로 전체 빌드한다.
- 빌드가 성공해야 작업 완료로 본다. 실패하면 오류를 고친 뒤 다시 빌드한다.

## 작업 끝에 보고할 것

1. 만든/수정한 파일 목록
2. 에디터에서 사람이 해야 할 일 (블루프린트 만들기, 위젯 디자인, 사운드 지정 등) — 단계별로
3. `TODO(기획 확인)` 목록
4. 빌드 결과
