# extra_script.py
Import("env")
import os
import shutil
from datetime import datetime
import json

def after_build(source, target, env):
    """Выполняется после сборки - переименовывает файлы"""
    print("📦 Running after_build...")
    
    # Читаем версию из уже сгенерированного Version.h
    version = read_version_from_header()
    
    # Переименовываем файлы
    rename_firmware_files(version, env)
    
    # Создаем информацию о сборке
    create_build_info(version, env)

def read_version_from_header():
    """Читает версию из Version.h"""
    version_h_path = "src/common/Version.h"
    
    if os.path.exists(version_h_path):
        with open(version_h_path, "r") as f:
            content = f.read()
            import re
            match = re.search(r'FIRMWARE_VERSION\s+"([^"]+)"', content)
            if match:
                return match.group(1)
    
    return datetime.now().strftime("%Y.%m.%d.%H%M")

def rename_firmware_files(version, env):
    """Переименовывает собранные файлы"""
    build_dir = env.subst("$BUILD_DIR")
    base_name = "webasto-controller"
    
    firmware_bin = os.path.join(build_dir, "firmware.bin")
    
    if os.path.exists(firmware_bin):
        # Версионный файл
        versioned_file = os.path.join(build_dir, f"{base_name}-v{version}.bin")
        shutil.copy2(firmware_bin, versioned_file)
        print(f"✅ Created versioned: {os.path.basename(versioned_file)}")
        
        # Latest симлинк
        latest_path = os.path.join(build_dir, f"{base_name}-latest.bin")
        if os.path.lexists(latest_path):
            os.remove(latest_path)
        os.symlink(os.path.basename(versioned_file), latest_path)
        print(f"🔗 Symlink: {base_name}-latest.bin")

def create_build_info(version, env):
    """Создает JSON с информацией о сборке"""
    build_dir = env.subst("$BUILD_DIR")
    
    build_info = {
        "version": version,
        "build_date": datetime.now().strftime("%Y-%m-%d"),
        "build_time": datetime.now().strftime("%H:%M:%S"),
        "build_timestamp": int(datetime.now().timestamp()),
        "output_files": []
    }
    
    # Добавляем информацию о созданных файлах
    for file in os.listdir(build_dir):
        if file.startswith("webasto-controller"):
            file_path = os.path.join(build_dir, file)
            if os.path.isfile(file_path):
                build_info["output_files"].append({
                    "name": file,
                    "size": os.path.getsize(file_path),
                    "modified": datetime.fromtimestamp(
                        os.path.getmtime(file_path)
                    ).strftime("%Y-%m-%d %H:%M:%S")
                })
    
    # Сохраняем
    info_path = os.path.join(build_dir, "build-info.json")
    with open(info_path, "w") as f:
        json.dump(build_info, f, indent=2)
    
    print(f"📝 Build info saved to: {os.path.basename(info_path)}")

def print_upload_info(source, target, env):
    """Выводит информацию перед загрузкой"""
    version = read_version_from_header()
    print(f"\n🚀 Uploading firmware v{version}")
    print(f"📦 Upload target: {target[0].name}")
    print(f"📁 Source: {source[0].get_abspath()}")

# Регистрируем обработчики
env.AddPostAction("buildprog", after_build)
env.AddPreAction("upload", print_upload_info)

print("✅ extra_script.py loaded")