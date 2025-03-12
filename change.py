def multiply_numbers_in_file(input_file, output_file):
    with open(input_file, 'r') as infile:
        lines = infile.readlines()

    with open(output_file, 'w') as outfile:
        for line in lines:
            parts = line.strip().split()
            if len(parts) == 2:
                try:
                    string_value, int_value = parts[0], float(parts[1])
                    new_value = int_value *12
                    outfile.write(f"{string_value} {new_value:.0f}\n")
                except ValueError:
                    # Handle invalid lines (e.g., missing or non-numeric values)
                    pass
            elif len(parts) == 3:
                try:
                    string_value, int_value, other_value = parts[0], float(parts[1]), int(parts[2])
                    new_value = int_value *12
                    outfile.write(f"{string_value} {new_value:.0f} {other_value}\n")
                except ValueError:
                    # Handle invalid lines (e.g., missing or non-numeric values
                    pass
# Example usage                                                                           
input_filename = 'output.txt'     
output_filename = 'vgg-tiny'
multiply_numbers_in_file(input_filename, output_filename)
print(f"Numbers multiplied and saved to {output_filename}")
