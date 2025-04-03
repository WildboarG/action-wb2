import hashlib
import struct
import sys


def add_header(input_file: str, output_file: str):
    """
    读取固件文件，计算 MD5，并添加固定格式的头部信息后生成新固件。
    
    头部结构：
    - 5 字节: 版本号 (填充到 5 字节)
    - 4 字节: 芯片类型
    - 32 字节: 固件 MD5
    - 128 字节: 下载 URL
    """

    VERSION = b"1.0.0" + b"\x00" * (5 - len("1.0.0"))  # 版本号填充到 5 字节
    CHIP_TYPE = b"0004"  # 4 字节芯片类型
    URL = b"http://192.168.0.14:8000/test.bin".ljust(128, b"\x00")  # 128 字节 URL

    try:
        with open(input_file, "rb") as f:
            firmware_data = f.read()
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        return
    except IOError:
        print(f"Error: Failed to read '{input_file}'.")
        return

    md5_hash = hashlib.md5(firmware_data).hexdigest().upper().encode()  # 计算 MD5

    # 确保 MD5 长度为 32 字节
    if len(md5_hash) != 32:
        print("Error: MD5 hash length mismatch!")
        return

    # 头部总长 169 字节
    header = struct.pack("5s4s32s128s", VERSION, CHIP_TYPE, md5_hash, URL)

    try:
        with open(output_file, "wb") as f:
            f.write(header)
            f.write(firmware_data)
    except IOError:
        print(f"Error: Failed to write '{output_file}'.")
        return

    print(f"Generated '{output_file}' with embedded header!")
    print(f"Original firmware MD5: {md5_hash.decode()}")


def main():
    """ 主函数，解析命令行参数并调用 `add_header` """
    if len(sys.argv) != 3:
        print("Usage: python3 addheader.py <input_file> <output_file>")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    add_header(input_file, output_file)


if __name__ == "__main__":
    main()

