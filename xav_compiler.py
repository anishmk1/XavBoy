import sys
import os

print ("Converting file from hex to binary")

def hex_to_binary(input_file):
    # Open the input file containing hex strings
    with open(input_file, 'r') as hex_file:
        output_file = os.path.splitext(input_file)[0] + '.gb'
        # Open the output file to write the raw binary data
        with open(output_file, 'wb') as bin_file:
            # Read each line from the hex file
            for line in hex_file:
                # Remove any leading/trailing whitespace characters
                line = line.strip()

                # Ignore empty lines or lines that are only comments
                if not line or line.startswith('#') or line.startswith('//'):
                    continue
                
                # Remove any part of the line after a comment (//) or (#)
                line = line.split('//', 1)[0].strip()
                line = line.split('#', 1)[0].strip()

                # Convert the hex string to binary (bytes)
                try:
                    binary_data = bytes.fromhex(line)
                    # Write the binary data to the output file
                    bin_file.write(binary_data)
                except ValueError:
                    # If conversion fails (e.g., invalid hex), skip this line
                    print(f"Warning: Skipping invalid hex line: {line}")

    print(f"Successfully converted hex to binary and saved in '{output_file}'.")

def main():
    # Check if the correct number of arguments is provided
    if len(sys.argv) != 2:
        print("Usage: python3 hex_to_bin.py <input_file> <output_file>")
        sys.exit(1)

    # Get input and output file from command-line arguments
    input_file = sys.argv[1]

    # Call the hex_to_binary function with the provided arguments
    hex_to_binary(input_file)



if __name__ == "__main__":
    main()
