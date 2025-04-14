import sys
def crc16_ccitt(seed: int, data: bytes) -> int:
    for byte in data:
        e = (seed ^ byte) & 0xFF
        f = (e ^ ((e << 4) & 0xFF)) & 0xFF
        seed = ((seed >> 8) ^
                ((f << 8) & 0xFFFF) ^
                ((f << 3) & 0xFFFF) ^
                (f >> 4)) & 0xFFFF
    return seed

def crc_from_file(filepath: str) -> int:
    try:
        with open(filepath, "rb") as file:
            file_data = file.read()
            return crc16_ccitt(0x0000, file_data)
    except FileNotFoundError:
        print(f"File not found: {filepath}")
        return -1

if __name__ == "__main__":
    file_path = sys.argv[1]  # Replace with your file
    crc = crc_from_file(file_path)
    if crc != -1:
        print(f"{crc:04X}")
