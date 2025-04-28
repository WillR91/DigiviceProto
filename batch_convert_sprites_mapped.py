# Python Script: batch_convert_sprites_mapped.py (Reads Char_Num.png, writes Char_Action_Num.*)
import os
import glob
from PIL import Image
import re
import sys

# --- Configuration ---
input_folder = "separated_sprites_input"
output_folder_absolute = r"Z:\DigiviceProto\assets" # <<< VERIFY THIS PATH
animation_mapping = {
    0: ("Idle", 0), 1: ("Idle", 1), 2: ("Walk", 0), 3: ("Walk", 1),
    4: ("Run", 0), 5: ("Run", 1), 6: ("Happy", 0), 7: ("Rest", 0),
    8: ("Attack", 0), 9: ("Turn", 0)
}
key_color_rgb = (255, 0, 255)
alpha_threshold = 128
# --- Helper ---
def sanitize_for_c(name_part):
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name_part)
    if name and name[0].isdigit(): name = "_" + name
    name = name.strip('_')
    name = re.sub(r'_{2,}', '_', name)
    if not name: name = "_"
    return name
# --- Main ---
try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)
    if not os.path.isdir(input_dir_path):
         print(f"Error: Input directory not found: {input_dir_path}", file=sys.stderr)
         sys.exit(1)
    if not os.path.isdir(output_folder_absolute):
        print(f"Output directory not found, attempting to create: {output_folder_absolute}")
        try: os.makedirs(output_folder_absolute, exist_ok=True)
        except OSError as e:
            print(f"Error: Could not create output directory: {output_folder_absolute}\n{e}", file=sys.stderr)
            sys.exit(1)
    else: print(f"Output directory found: {output_folder_absolute}")
    print(f"Input folder: {input_dir_path}")
    print(f"Output folder: {output_folder_absolute}")
    search_pattern = os.path.join(input_dir_path, "*.png")
    png_files = glob.glob(search_pattern)
    if not png_files: print(f"Warning: No PNG files found in '{input_dir_path}' matching '*.png'")
    else: print(f"Found {len(png_files)} PNG files. Starting conversion...")
    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3)
    processed_count = 0
    error_count = 0
    for image_path in png_files:
        base_filename = os.path.basename(image_path)
        match = re.match(r'^(.*)_(\d+)\.png$', base_filename, re.IGNORECASE)
        if not match:
            print(f"  Skipping '{base_filename}': Does not match 'Name_Number.png' format.", file=sys.stderr)
            error_count += 1; continue
        character_name_raw = match.group(1)
        try: frame_number = int(match.group(2))
        except ValueError:
            print(f"  Skipping '{base_filename}': Frame number part '{match.group(2)}' not integer.", file=sys.stderr)
            error_count += 1; continue
        if frame_number in animation_mapping:
            action_name_raw, action_frame_index = animation_mapping[frame_number]
            character_name_sanitized = sanitize_for_c(character_name_raw).capitalize(); action_name_sanitized = sanitize_for_c(action_name_raw).capitalize()
            if not character_name_sanitized: character_name_sanitized = "Sprite"
            if not action_name_sanitized: action_name_sanitized = "Action"
            define_prefix = f"{character_name_sanitized.upper()}_{action_name_sanitized.upper()}_{action_frame_index}"; variable_name = f"{character_name_sanitized}_{action_name_sanitized}_{action_frame_index}_data"
            output_base_name = f"{character_name_sanitized}_{action_name_sanitized}_{action_frame_index}"; output_h_filename = f"{output_base_name}.h"
            output_path = os.path.join(output_folder_absolute, output_h_filename); include_guard = f"{define_prefix}_H"
            print(f"\nProcessing '{base_filename}' ==>"); print(f"  -> Action: {action_name_raw}, Frame: {action_frame_index}"); print(f"  -> Output File: '{output_h_filename}'"); print(f"  -> Variable: '{variable_name}', Defines: '{define_prefix}_WIDTH/HEIGHT'")
            try:
                img = Image.open(image_path); img = img.convert('RGBA') if img.mode != 'RGBA' else img; width, height = img.size; pixels_rgba = list(img.getdata())
                print(f"  Image size: {width}x{height}")
                with open(output_path, "w") as f:
                    f.write(f"// Converted from {base_filename}\n"); f.write(f"// Action: {action_name_raw}, Frame Index: {action_frame_index}\n"); f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n\n")
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
        else: print(f"  Skipping '{base_filename}': Frame number {frame_number} not found in animation_mapping.", file=sys.stderr); error_count +=1
    print(f"\nBatch conversion finished. Processed {processed_count} files. Encountered {error_count} errors or skipped files.")
except Exception as e: print(f"A critical error occurred: {e}", file=sys.stderr); sys.exit(1)