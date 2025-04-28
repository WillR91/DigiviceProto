# Python Script: convert_generic.py
# Converts any PNG in input folder to a .h file based on filename.
import os
import glob
from PIL import Image
import re
import sys

# --- Configuration ---
input_folder = "generic_input"
output_folder_absolute = r"Z:\DigiviceProto\assets" # <<< VERIFY THIS PATH
key_color_rgb = (255, 0, 255)
alpha_threshold = 128
# --- Helper ---
def sanitize_for_c(name_part):
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name_part)
    if name and name[0].isdigit(): name = "_" + name
    name = name.strip('_'); name = re.sub(r'_{2,}', '_', name)
    if not name: name = "_"
    return name
# --- Main ---
try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)
    if not os.path.isdir(input_dir_path):
         print(f"Error: Input directory not found: {input_dir_path}", file=sys.stderr); sys.exit(1)
    if not os.path.isdir(output_folder_absolute):
        print(f"Output directory not found, attempting to create: {output_folder_absolute}")
        try: os.makedirs(output_folder_absolute, exist_ok=True)
        except OSError as e: print(f"Error: Could not create output directory: {output_folder_absolute}\n{e}", file=sys.stderr); sys.exit(1)
    else: print(f"Output directory found: {output_folder_absolute}")
    print(f"Looking for generic PNGs in: {input_dir_path}"); print(f"Output folder: {output_folder_absolute}")
    search_pattern = os.path.join(input_dir_path, "*.png"); png_files = glob.glob(search_pattern)
    if not png_files: print(f"Warning: No PNG files found in '{input_dir_path}' matching '*.png'")
    else: print(f"Found {len(png_files)} PNG files. Starting conversion...")
    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3)
    processed_count = 0; error_count = 0
    for image_path in png_files:
        base_filename = os.path.basename(image_path); name_part_raw = os.path.splitext(base_filename)[0]
        sanitized_base_var = sanitize_for_c(name_part_raw).capitalize()
        if not sanitized_base_var: sanitized_base_var = "Generic"
        define_prefix = sanitized_base_var.upper(); variable_name = f"{sanitized_base_var}_data"
        output_h_filename = f"{sanitized_base_var}.h"; output_path = os.path.join(output_folder_absolute, output_h_filename)
        include_guard = f"{define_prefix}_H"
        print(f"\nProcessing '{base_filename}' ==>"); print(f"  -> Output File: '{output_h_filename}'")
        try:
            img = Image.open(image_path); img = img.convert('RGBA') if img.mode != 'RGBA' else img; width, height = img.size; pixels_rgba = list(img.getdata())
            print(f"  Image size: {width}x{height}")
            with open(output_path, "w") as f:
                f.write(f"// Converted from {base_filename}\n"); f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n\n")
                f.write(f"#ifndef {include_guard}\n"); f.write(f"#define {include_guard}\n\n")
                f.write("#include <cstdint>\n\n") # <<< CORRECTED INCLUDE
                f.write(f"#define {define_prefix}_WIDTH {width}\n"); f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")
                f.write(f"// RGB565 format pixel data\n"); f.write(f"const uint16_t {variable_name}[] = {{\n  ")
                count = 0; num_pixels = len(pixels_rgba)
                for i, p in enumerate(pixels_rgba):
                    r, g, b, a = p
                    rgb565 = key_color_565 if a < alpha_threshold else (((max(0, min(r, 255)) >> 3) << 11) | ((max(0, min(g, 255)) >> 2) << 5) | (max(0, min(b, 255)) >> 3))
                    f.write(f"0x{rgb565:04X}"); f.write("," if i < num_pixels - 1 else "")
                    count += 1
                    if count % 12 == 0 or i == num_pixels - 1: f.write("\n"); f.write("  " if i < num_pixels - 1 else "")
                    elif i < num_pixels - 1: f.write(" ")
                f.write(f"}}; // End of {variable_name}\n\n"); f.write(f"#endif // {include_guard}\n")
            print(f"  Successfully generated '{output_h_filename}'"); processed_count += 1
        except Exception as e: print(f"  ERROR processing '{base_filename}': {e}", file=sys.stderr); error_count += 1
    print(f"\nGeneric conversion finished. Processed {processed_count} files. Encountered {error_count} errors or skipped files.")
except Exception as e: print(f"A critical error occurred: {e}", file=sys.stderr); sys.exit(1)