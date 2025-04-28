# Python Script: batch_convert_sprites_mapped.py (Reads Char_Num.png, writes Char_Action_Num.*)
import os
import glob
from PIL import Image
import re
import sys # <<< Add for stderr

# --- Configuration ---

# Folder containing your separated PNGs named like "CharacterName_FrameNumber.png"
# This folder should be in the same directory as this script.
input_folder = "separated_sprites_input" # <<< ADJUST if your input folder name is different

# Folder where the output .h files will be saved
# Should point correctly to your project's assets folder
# Example assumes assets folder is ONE level up from script directory
# and then inside the project folder (e.g. DigiviceProto/assets)
# ADJUST THIS RELATIVE PATH TO MATCH YOUR STRUCTURE
output_folder = "Z:\DigiviceProto\assets" # <<< EXAMPLE PATH, ADJUST

# --- Animation Mapping ---
# Maps the input frame index (0-9) to a tuple: (ActionName, ActionFrameIndex)
animation_mapping = {
    0: ("Idle", 0),
    1: ("Idle", 1),
    2: ("Walk", 0),
    3: ("Walk", 1),
    4: ("Run", 0),  # Example, might need different mapping
    5: ("Run", 1),  # Example, might need different mapping
    6: ("Happy", 0),
    7: ("Rest", 0),
    8: ("Attack", 0),
    9: ("Turn", 0)
}
# ---

# Magenta Color Key for Transparency
key_color_rgb = (255, 0, 255)
alpha_threshold = 128
# ---------------------

# --- Helper to sanitize for C names ---
def sanitize_for_c(name_part):
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name_part)
    if name and name[0].isdigit():
        name = "_" + name
    # Prevent names starting/ending with underscore or double underscores
    name = name.strip('_')
    name = re.sub(r'_{2,}', '_', name)
    return name.upper() # Conventionally, defines/macros are uppercase
# ----------------------------------------------------------

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)
    # Construct absolute output path for clarity
    output_dir_path_relative = os.path.join(script_dir, output_folder)
    output_dir_path_absolute = os.path.abspath(output_dir_path_relative)

    if not os.path.isdir(input_dir_path):
         print(f"Error: Input directory not found: {input_dir_path}", file=sys.stderr)
         sys.exit(1)

    os.makedirs(output_dir_path_absolute, exist_ok=True) # Use absolute path
    print(f"Input folder: {input_dir_path}")
    print(f"Output folder: {output_dir_path_absolute}") # Print absolute path

    search_pattern = os.path.join(input_dir_path, "*.png")
    png_files = glob.glob(search_pattern)

    if not png_files:
        print(f"Warning: No PNG files found in '{input_dir_path}' matching '*.png'")
    else:
        print(f"Found {len(png_files)} PNG files. Starting conversion...")

    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3)

    processed_count = 0
    error_count = 0

    for image_path in png_files:
        base_filename = os.path.basename(image_path)

        # Parse filename like "CharacterName_FrameNumber.png"
        match = re.match(r'^(.*)_(\d+)\.png$', base_filename, re.IGNORECASE) # Ignore case
        if not match:
            print(f"  Skipping '{base_filename}': Does not match 'Name_Number.png' format.", file=sys.stderr)
            continue

        character_name_raw = match.group(1)
        try:
            frame_number = int(match.group(2))
        except ValueError:
            print(f"  Skipping '{base_filename}': Frame number part '{match.group(2)}' not integer.", file=sys.stderr)
            continue

        # Look up action and new index
        if frame_number in animation_mapping:
            action_name_raw, action_frame_index = animation_mapping[frame_number]

            # --- Sanitize names for C/Filesystem ---
            # Keep original case for variable names, uppercase for defines
            character_name_var = sanitize_for_c(character_name_raw).capitalize() # e.g., Agumon
            action_name_var = sanitize_for_c(action_name_raw).capitalize()       # e.g., Idle
            define_prefix = f"{character_name_var.upper()}_{action_name_var.upper()}_{action_frame_index}" # e.g., AGUMON_IDLE_0
            variable_name = f"{character_name_var}_{action_name_var}_{action_frame_index}_data" # e.g., Agumon_Idle_0_data
            output_base_name = f"{character_name_var}_{action_name_var}_{action_frame_index}" # e.g., Agumon_Idle_0
            output_h_filename = f"{output_base_name}.h"
            output_path = os.path.join(output_dir_path_absolute, output_h_filename) # Use absolute path
            include_guard = f"{define_prefix}_H" # Define include guard name
            # ---------------------------------------------

            print(f"\nProcessing '{base_filename}' ==>")
            print(f"  -> Action: {action_name_raw}, Frame: {action_frame_index}")
            print(f"  -> Output File: '{output_h_filename}'")
            print(f"  -> Variable: '{variable_name}', Defines: '{define_prefix}_WIDTH/HEIGHT'")

            try:
                # --- Image processing ---
                img = Image.open(image_path)
                if img.mode != 'RGBA':
                    img = img.convert('RGBA')
                width, height = img.size
                pixels_rgba = list(img.getdata())
                print(f"  Image size: {width}x{height}")

                # --- File writing ---
                with open(output_path, "w") as f:
                    # --- Start Header File ---
                    f.write(f"// Converted from {base_filename}\n")
                    f.write(f"// Action: {action_name_raw}, Frame Index: {action_frame_index}\n")
                    f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n\n")

                    # --- Include Guard Start ---
                    f.write(f"#ifndef {include_guard}\n")
                    f.write(f"#define {include_guard}\n\n")

                    # --- Include Standard Integer Types --- <<<< CORRECT INCLUDE
                    f.write("#include <cstdint>\n\n")

                    # --- Defines for Width and Height ---
                    f.write(f"#define {define_prefix}_WIDTH {width}\n")
                    f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")

                    # --- Array Definition --- <<<< CORRECTED SYNTAX
                    f.write(f"// RGB565 format pixel data\n")
                    f.write(f"const uint16_t {variable_name}[] = {{\n  ") # Use [] for auto-sizing

                    # --- Pixel Data Loop (Same as before) ---
                    count = 0
                    for i, p in enumerate(pixels_rgba):
                        r, g, b, a = p
                        if a < alpha_threshold:
                            rgb565 = key_color_565
                        else:
                            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

                        f.write(f"0x{rgb565:04X}")
                        if i < len(pixels_rgba) - 1: f.write(",")
                        count += 1
                        if count % 12 == 0: # Newline every 12 pixels
                             f.write("\n  ")
                        elif i < len(pixels_rgba) - 1:
                            f.write(" ") # Space between pixels on same line

                    # --- End Array and Include Guard ---
                    f.write("\n}}; // End of {variable_name}\n\n") # Closing brace and semicolon for array
                    f.write(f"#endif // {include_guard}\n") # Include guard end

                print(f"  Successfully generated '{output_h_filename}'")
                processed_count += 1

            except Exception as e:
                print(f"  ERROR processing '{base_filename}': {e}", file=sys.stderr)
                error_count += 1
        else:
            print(f"  Skipping '{base_filename}': Frame number {frame_number} not found in animation_mapping.", file=sys.stderr)
            error_count +=1


    print(f"\nBatch conversion finished. Processed {processed_count} files. Encountered {error_count} errors or skipped files.")

except Exception as e:
    print(f"A critical error occurred: {e}", file=sys.stderr)
    sys.exit(1) # Exit with error code if critical error