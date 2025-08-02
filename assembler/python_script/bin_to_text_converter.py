import sys

def bin_to_text_bytewise(input_file, output_file):
    with open(input_file, "rb") as f:
        data = f.read()

    with open(output_file, "w") as f_out:
        for byte in data:
            binary_str = format(byte, '08b')  # 8-bit binary
            f_out.write(binary_str + "\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python bin_to_text.py <input_file> <output_file>")
        sys.exit(1)

    bin_to_text_bytewise(sys.argv[1], sys.argv[2])

