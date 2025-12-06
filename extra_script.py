Import("env")
import os
import shutil
from datetime import datetime
import re

def get_firmware_version():
    # 1. Переменная окружения (самый высокий приоритет)
    env_version = os.getenv("FIRMWARE_VERSION")
    if env_version:
        print(f"📦 Using environment variable: {env_version}")
        return env_version
    
    # 2. .env файл
    try:
        env_file = ".env"
        if os.path.exists(env_file):
            with open(env_file, "r") as f:
                content = f.read()
                # Ищем FIRMWARE_VERSION
                match = re.search(r'FIRMWARE_VERSION\s*=\s*["\']?([^"\'\s]+)["\']?', content)
                if match:
                    version = match.group(1)
                    print(f"📦 Using .env file: {version}")
                    return version
    except Exception as e:
        print(f"⚠️  Error reading .env: {e}")
    
    # 3. Version.h файл
    try:
        version_file = "src/common/Version.h"
        if os.path.exists(version_file):
            with open(version_file, "r") as f:
                content = f.read()
                match = re.search(r'FIRMWARE_VERSION\s+"([^"]+)"', content)
                if match:
                    version = match.group(1)
                    print(f"📦 Using Version.h: {version}")
                    return version
    except Exception as e:
        print(f"⚠️  Error reading Version.h: {e}")
    
    # 4. Значение по умолчанию
    default_version = "1.0.0"
    print(f"⚠️  Using default version: {default_version}")
    return default_version

def generate_version_header(version):
    version_header = f"""#ifndef FIRMWARE_VERSION_H
#define FIRMWARE_VERSION_H

// Автоматически сгенерированная версия из .env
#define FIRMWARE_VERSION "{version}"
#define FIRMWARE_BUILD_DATE "{datetime.now().strftime('%Y-%m-%d')}"
#define FIRMWARE_BUILD_TIME "{datetime.now().strftime('%H:%M:%S')}"

#endif
"""

    os.makedirs("src/common", exist_ok=True)

    with open("src/common/Version.h", "w") as f:
        f.write(version_header)
    
    print(f"✅ Generated Version.h with version: {version}")

def before_build(source, target, env):
    print("🔧 Configuring build...")

    firmware_version = get_firmware_version()

    generate_version_header(firmware_version)

    env.Append(
        CPPDEFINES=[
            f"FIRMWARE_VERSION=\\\"{firmware_version}\\\""
        ]
    )
    
    print(f"🎯 Building firmware v{firmware_version}")
    """Выполняется после сборки - переименовывает файлы"""
    print("📦 Renaming firmware files...")
    
    # Получаем версию
    firmware_version = get_firmware_version()
    
    # Переименовываем файлы
    for t in target:
        if str(t).endswith(".bin"):
            original = t.get_abspath()
            directory = os.path.dirname(original)
            
            # Основное имя файла
            base_name = "webasto-controller"
            new_name = f"{base_name}-v{firmware_version}.bin"
            new_path = os.path.join(directory, new_name)
            
            # Копируем с новым именем
            shutil.copy2(original, new_path)
            print(f"✅ Created: {new_name}")
            
            # Также создаем копию с timestamp
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            timestamp_name = f"{base_name}-v{firmware_version}-{timestamp}.bin"
            timestamp_path = os.path.join(directory, timestamp_name)
            shutil.copy2(original, timestamp_path)
            
            print(f"📁 Backup: {timestamp_name}")
            
            # Создаем симлинк на latest.bin
            latest_path = os.path.join(directory, f"{base_name}-latest.bin")
            if os.path.exists(latest_path):
                os.remove(latest_path)
            os.symlink(new_path, latest_path)
            print(f"🔗 Symlink: {base_name}-latest.bin -> {new_name}")

def print_upload_info(source, target, env):
    firmware_version = get_firmware_version()
    print(f"🚀 Uploading firmware v{firmware_version}")
    print(f"📦 Upload target: {target[0].name}")
    print(f"📁 Source: {source[0]}")

# Регистрируем обработчики
env.AddPreAction("buildprog", before_build)
env.AddPreAction("upload", print_upload_info)