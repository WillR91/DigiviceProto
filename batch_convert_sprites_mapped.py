# Python Script: batch_convert_sprites_mapped.py (Reads Char_Num.png, writes Char_Action_Num.*)
import os
import glob
from PIL import Image
import re
import sys

# --- Configuration ---

# Folder containing your separated PNGs named like "CharacterName_FrameNumber.png"
# This folder should be in the same directory as this script.
input_folder = "separated_sprites_input" # <<< ADJUST if your input folder name is different

# --- IMPORTANT PATH CONFIGURATION ---
# Define the *absolute* path to your C++ project's assets folder
# Use forward slashes or raw strings for Windows paths
# Example: output_folder_absolute = r"Z:\DigiviceProto\assets"
# Example: output_folder_absolute = "Z:/DigiviceProto/assets"
# Make sure this path is CORRECT for your system
output_folder_absolute = r"Z:\DigiviceProto\assets" # <<< USE RAW STRING FOR WINDOWS PATH
# -------------------------------------


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
    # Ensure name is not empty after sanitization
    if not name:
        name = "_"
    return name # Return mixed case for variable prefix
# ----------------------------------------------------------

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)

    # Validate input directory
    if not os.path.isdir(input_dir_path):
         print(f"Error: Input directory not found: {input_dir_path}", file=sys.stderr)
         sys.exit(1)

    # Validate and create output directory (using the absolute path directly)
    if not os.path.isdir(output_folder_absolute):
        print(f"Output directory not found, attempting to create: {output_folder_absolute}")
        try:
            os.makedirs(output_folder_absolute, exist_ok=True)
        except OSError as e:
            print(f"Error: Could not create output directory: {output_folder_absolute}\n{e}", file=sys.stderr)
            sys.exit(1)
    else:
        print(f"Output directory found: {output_folder_absolute}")


    print(f"Input folder: {input_dir_path}")
    print(f"Output folder: {output_folder_absolute}")

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
            error_count += 1 # Count skipped file as an error/issue
            continue

        character_name_raw = match.group(1)
        try:
            frame_number = int(match.group(2))
        except ValueError:
            print(f"  Skipping '{base_filename}': Frame number part '{match.group(2)}' not integer.", file=sys.stderr)
            error_count += 1
            continue

        # Look up action and new index
        if frame_number in animation_mapping:
            action_name_raw, action_frame_index = animation_mapping[frame_number]

            # --- Sanitize names for C/Filesystem ---
            # Keep original case for variable names (more readable), uppercase for defines
            character_name_sanitized = sanitize_for_c(character_name_raw).capitalize() # e.g., Agumon
            action_name_sanitized = sanitize_for_c(action_name_raw).capitalize()       # e.g., Idle
            # Ensure names are not empty after sanitize_for_c might strip underscores
            if not character_name_sanitized: character_name_sanitized = "Sprite"
            if not action_name_sanitized: action_name_sanitized = "Action"

            define_prefix = f"{character_name_sanitized.upper()}_{action_name_sanitized.upper()}_{action_frame_index}" # e.g., AGUMON_IDLE_0
            variable_name = f"{character_name_sanitized}_{action_name_sanitized}_{action_frame_index}_data" # e.g., Agumon_Idle_0_data
            output_base_name = f"{character_name_sanitized}_{action_name_sanitized}_{action_frame_index}" # e.g., Agumon_Idle_0
            output_h_filename = f"{output_base_name}.h"
            output_path = os.path.join(output_folder_absolute, output_h_filename) # Use absolute path
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

                    # --- Include Standard Integer Types ---
                    f.write("#include <cstdint>\n\n")

                    # --- Defines for Width and Height ---
                    f.write(f"#define {define_prefix}_WIDTH {width}\n")
                    f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")

                    # --- Array Definition ---
                    f.write(f"// RGB565 format pixel data\n")
                    f.write(f"const uint16_t {variable_name}[] = {{\n  ") # Use [] for auto-sizing

                    # --- Pixel Data Loop ---
                    count = 0
                    num_pixels = len(pixels_rgba)
                    for i, p in enumerate(pixels_rgba):
                        r, g, b, a = p
                        if a < alpha_threshold:
                            rgb565 = key_color_565
                        else:
                            # Clamp values just in case (though PIL usually handles this)
                            r_clamped = max(0, min(r, 255))
                            g_clamped = max(0, min(g, 255))
                            b_clamped = max(0, min(b, 255))
                            rgb565 = ((r_clamped >> 3) << 11) | ((g_clamped >> 2) << 5) | (b_clamped >> 3)

                        f.write(f"0x{rgb565:04X}")
                        if i < num_pixels - 1: f.write(",") # Add comma if not the last pixel
                        count += 1
                        # Formatting: Newline every 12 pixels or if it's the last pixel
                        if count % 12 == 0 or i == num_pixels - 1:
                             f.write("\n")
                             if i < num_pixels - 1: # Add indentation if not the very last line
                                 f.write("  ")
                        elif i < num_pixels - 1: # Add space if not last pixel and not end of line
                            f.write(" ")

                    # --- End Array and Include Guard ---
                    # Use f-string correctly for variable name in comment
                    f.write(f"}}; // End of {variable_name}\n\n")
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