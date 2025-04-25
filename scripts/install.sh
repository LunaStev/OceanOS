#!/bin/bash

set -e

echo "🔧 OceanOS 개발 환경 설치 스크립트 실행"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo "❌ 지원되지 않는 운영체제입니다."
    exit 1
fi

echo "🌍 OS 감지됨: $OS"

install_packages() {
    echo "📦 필수 패키지 설치 중..."
    if [ "$OS" == "linux" ]; then
        sudo apt update
        sudo apt install -y clang gcc make binutils llvm lld \
                            qemu qemu-system-x86 gdb ovmf \
                            gnu-efi efibootmgr \
                            mtools dosfstools e2fsprogs \
                            cmake ninja-build \
                            libpcap-dev
    elif [ "$OS" == "macos" ]; then
        brew update
        brew install llvm make binutils qemu gdb \
                     mtools dosfstools e2fsprogs \
                     cmake ninja libpcap
    fi
}

# QEMU 설정
setup_qemu() {
    echo "⚙️ QEMU 설정 중..."
    mkdir -p ~/qemu
    if [ "$OS" == "linux" ]; then
        cp /usr/share/ovmf/OVMF.fd ~/qemu/OVMF.fd
    elif [ "$OS" == "macos" ]; then
        echo "macOS에서 OVMF를 수동으로 다운로드하세요."
    fi
}

# 빌드 테스트
test_build() {
    echo "🛠️ 빌드 테스트 중..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
}

# 설치 실행
install_packages
setup_qemu
test_build

echo "✅ OceanOS 개발 환경 설치 완료!"
echo "💡 QEMU 실행: ./scripts/qemu_run.sh"