# OceanOS

OceanOS는 UEFI 기반의 현대적인 아키텍처만을 지원하는 완전한 GUI 운영체제입니다. BIOS나 레거시 기술을 완전히 배제하고, 현대적인 드라이버와 저장소 지원을 목표로 합니다.

## 🎯 목표

- **UEFI 전용 부팅 및 운영**
- **BIOS나 레거시 하드웨어 지원 제외**
- **현대적인 드라이버 및 저장소 지원** (NVMe, PCIe, USB 등)
- **완전한 GUI 데스크톱 환경**
- **멀티태스킹 및 멀티스레딩 지원**
- **현대적인 메모리 관리 시스템**

## 🚀 주요 기능

### 시스템 구성요소
- **UEFI 부트로더**: 현대적인 UEFI 펌웨어 지원
- **커널**: 64비트 x86_64 아키텍처 지원
- **메모리 관리**: 페이징, 가상 메모리, 메모리 보호
- **스케줄러**: 멀티태스킹, 우선순위 기반 스케줄링
- **GUI 시스템**: 윈도우 관리, 데스크톱 환경
- **셸**: 명령줄 인터페이스, 내장 명령어 지원

### 드라이버 지원
- **디스플레이**: 프레임버퍼 기반 그래픽 출력
- **키보드**: PS/2 키보드 입력 처리
- **마우스**: PS/2 마우스 지원 (계획)
- **네트워크**: 이더넷, Wi-Fi 지원 (계획)
- **저장소**: NVMe, USB 저장소 지원 (계획)

### 사용자 인터페이스
- **데스크톱**: 윈도우 기반 GUI 환경
- **태스크바**: 실행 중인 프로그램 관리
- **시작 메뉴**: 프로그램 실행 및 시스템 설정
- **터미널**: 명령줄 셸 인터페이스

## 🛠️ 빌드 및 실행

### 필수 요구사항
- Linux, macOS, 또는 WSL 환경
- GCC 컴파일러 (크로스 컴파일러 포함)
- QEMU (가상화 테스트용)
- xorriso (ISO 이미지 생성용)

### 의존성 설치

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc-multilib gcc-mingw-w64 \
    binutils-mingw-w64 xorriso ovmf qemu-system-x86
```

#### Arch Linux
```bash
sudo pacman -S --needed base-devel mingw-w64-gcc mingw-w64-binutils \
    xorriso edk2-ovmf qemu
```

#### macOS
```bash
brew install gcc make xorriso qemu
```

### 빌드 방법

1. **저장소 클론**
```bash
git clone https://github.com/yourusername/OceanOS.git
cd OceanOS
```

2. **도구 확인**
```bash
make check-tools
```

3. **빌드**
```bash
make all
```

4. **ISO 이미지 생성**
```bash
make iso
```

5. **QEMU에서 실행**
```bash
make qemu-uefi
```

### 빌드 옵션

- `make debug`: 디버그 심볼 포함 빌드
- `make release`: 최적화된 릴리즈 빌드
- `make clean`: 빌드 디렉토리 정리

## 📁 프로젝트 구조

```
OceanOS/
├── bootloader/          # UEFI 부트로더
│   ├── main.c          # 부트로더 메인
│   ├── uefi.h          # UEFI 헤더
│   ├── framebuffer.h   # 프레임버퍼 정보
│   └── link.ld         # 부트로더 링커 스크립트
├── kernel/             # 커널
│   ├── main.c          # 커널 메인
│   ├── kernel.h        # 커널 헤더
│   ├── memory/         # 메모리 관리
│   ├── drivers/        # 하드웨어 드라이버
│   ├── scheduler/      # 프로세스 스케줄러
│   └── link.ld         # 커널 링커 스크립트
├── libs/               # 라이브러리
│   ├── types.h         # 기본 타입 정의
│   ├── stdio.h         # 표준 입출력
│   └── stdio.c         # 입출력 구현
├── userland/           # 사용자 공간
│   ├── gui/            # GUI 시스템
│   ├── shell/          # 셸 시스템
│   └── init/           # 초기화 프로그램
├── scripts/            # 빌드 스크립트
├── Makefile            # 메인 빌드 파일
└── README.md           # 프로젝트 문서
```

## 🎮 사용법

### 부팅 후 사용 가능한 기능

1. **GUI 데스크톱**
   - 태스크바의 "Start" 버튼 클릭
   - 윈도우 생성 및 관리
   - 마우스 및 키보드 입력

2. **터미널 셸**
   - 키보드로 명령어 입력
   - 내장 명령어 사용

### 사용 가능한 명령어

- `help`: 사용 가능한 명령어 목록
- `clear`: 화면 지우기
- `echo`: 텍스트 출력
- `ls`: 디렉토리 목록 (구현 예정)
- `cd`: 디렉토리 변경 (구현 예정)
- `pwd`: 현재 디렉토리 출력
- `whoami`: 현재 사용자 정보
- `memory`: 메모리 정보 (구현 예정)
- `reboot`: 시스템 재부팅
- `shutdown`: 시스템 종료

## 🔧 개발 가이드

### 코드 스타일
- **K&R 스타일** 준수
- **snake_case** 함수 및 변수명
- **PascalCase** 타입 및 구조체명
- **ALL_CAPS** 매크로 및 상수

### 기여 방법
1. Fork 저장소
2. 기능 브랜치 생성
3. 코드 작성 및 테스트
4. Pull Request 생성

자세한 기여 가이드는 [CONTRIBUTING.md](CONTRIBUTING.md)를 참조하세요.

## 📋 개발 로드맵

### 완료된 기능 ✅
- [x] UEFI 부트로더
- [x] 기본 커널 구조
- [x] 메모리 관리 시스템
- [x] 디스플레이 드라이버
- [x] 키보드 입력 시스템
- [x] GUI 데스크톱 환경
- [x] 셸 시스템
- [x] 멀티태스킹 스케줄러

### 진행 중인 기능 🔄
- [ ] 파일 시스템 지원
- [ ] 네트워크 드라이버
- [ ] USB 지원
- [ ] 마우스 입력 처리

### 계획된 기능 📅
- [ ] 멀티미디어 지원
- [ ] 네트워킹 스택
- [ ] 보안 기능
- [ ] 패키지 관리 시스템
- [ ] 개발자 도구

## 🐛 알려진 문제

- 파일 시스템이 아직 구현되지 않음
- 네트워크 기능 미지원
- 일부 하드웨어 드라이버 미구현

## 📄 라이선스

이 프로젝트는 [MPL-2.0](LICENCE) 라이선스 하에 배포됩니다.


---

**OceanOS** - 현대적인 UEFI 기반 GUI 운영체제
