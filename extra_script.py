Import("env")

# Простая функция для отладки
def print_upload_info(source, target, env):
    print("🔧 Configuring upload...")
    print("📦 Upload target:", target[0])
    print("📁 Source:", source[0])

# Добавляем хук для отладки
env.AddPreAction("upload", print_upload_info)