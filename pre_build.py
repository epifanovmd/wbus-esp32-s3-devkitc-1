# pre_build.py
Import("env")
import os
import re
from datetime import datetime
import hashlib

def force_version_update():
    """Принудительно обновляет Version.h перед компиляцией"""
    print("🔧 PRE-BUILD: Checking version...")
    
    # 1. Получаем версию из .env
    def get_version_from_env():
        env_file = ".env"
        if os.path.exists(env_file):
            with open(env_file, "r") as f:
                content = f.read()
                match = re.search(r'FIRMWARE_VERSION\s*=\s*["\']?([^"\'\s]+)["\']?', content)
                if match:
                    return match.group(1)
        return None
    
    # 2. Проверяем хеш .env
    def get_env_hash():
        if not os.path.exists(".env"):
            return "no_env"
        with open(".env", "rb") as f:
            return hashlib.md5(f.read()).hexdigest()
    
    # 3. Сравниваем с текущим Version.h
    version_h_path = "src/common/Version.h"
    
    # Получаем новую версию
    new_version = get_version_from_env()
    if not new_version:
        new_version = datetime.now().strftime("%Y.%m.%d.%H%M")
    
    # Получаем старую версию (если файл существует)
    old_version = None
    if os.path.exists(version_h_path):
        with open(version_h_path, "r") as f:
            content = f.read()
            match = re.search(r'FIRMWARE_VERSION\s+"([^"]+)"', content)
            if match:
                old_version = match.group(1)
    
    # Получаем хеши для сравнения
    env_hash = get_env_hash()
    build_dir = env.subst("$BUILD_DIR")
    os.makedirs(build_dir, exist_ok=True)
    hash_file = os.path.join(build_dir, ".env_version.hash")
    
    old_hash = None
    if os.path.exists(hash_file):
        with open(hash_file, "r") as f:
            old_hash = f.read().strip()
    
    # Проверяем, нужно ли обновлять
    needs_update = False
    
    if old_version != new_version:
        print(f"🔁 Version changed: {old_version or 'none'} -> {new_version}")
        needs_update = True
    elif env_hash != old_hash:
        print(f"🔁 .env file changed (hash)")
        needs_update = True
    
    if needs_update:
        # Генерируем новый Version.h
        version_header = f"""#ifndef FIRMWARE_VERSION_H
#define FIRMWARE_VERSION_H

// Автоматически сгенерированная версия
// Сгенерировано: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
#define FIRMWARE_VERSION "{new_version}"
#define FIRMWARE_BUILD_DATE "{datetime.now().strftime('%Y-%m-%d')}"
#define FIRMWARE_BUILD_TIME "{datetime.now().strftime('%H:%M:%S')}"
#define FIRMWARE_BUILD_UNIX {int(datetime.now().timestamp())}

#endif
"""
        
        os.makedirs("src/common", exist_ok=True)
        with open(version_h_path, "w") as f:
            f.write(version_header)
        
        print(f"✅ Generated Version.h with version: {new_version}")
        
        # Сохраняем хеш
        with open(hash_file, "w") as f:
            f.write(env_hash)
        
        # Принудительно удаляем объектные файлы для пересборки
        print("🧹 Forcing rebuild...")
        clean_related_files(env)
    else:
        print(f"✅ Version unchanged: {new_version}")

def clean_related_files(env):
    """Удаляет файлы, связанные с Version.h для принудительной пересборки"""
    build_dir = env.subst("$BUILD_DIR")
    
    # Ищем все .o файлы, которые могут включать Version.h
    import glob
    obj_files = glob.glob(os.path.join(build_dir, "**", "*.o"), recursive=True)
    
    # Также удаляем файлы зависимостей
    dep_files = glob.glob(os.path.join(build_dir, "**", "*.d"), recursive=True)
    
    files_to_remove = obj_files + dep_files
    
    for file in files_to_remove:
        try:
            os.remove(file)
            # print(f"  Removed: {os.path.basename(file)}")
        except:
            pass
    
    # Удаляем основной elf/bin если они существуют
    elf_file = os.path.join(build_dir, "firmware.elf")
    bin_file = os.path.join(build_dir, "firmware.bin")
    
    for file in [elf_file, bin_file]:
        if os.path.exists(file):
            try:
                os.remove(file)
                print(f"  Removed: {os.path.basename(file)}")
            except:
                pass

# Выполняем ДО начала компиляции
force_version_update()

# Добавляем .env как зависимость для ВСЕХ целей
env.Depends("$BUILD_DIR/${PROGNAME}.elf", ".env")
env.Depends("$BUILD_DIR/${PROGNAME}.bin", ".env")

# Также добавляем Version.h как зависимость
env.Depends("$BUILD_DIR/${PROGNAME}.elf", "src/common/Version.h")