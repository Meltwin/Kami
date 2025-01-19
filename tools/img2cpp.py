# =============================================================================
# Images to C++ Header files
# Author: Meltwin (https://github.com/meltwin)
# Copyright: (c) Meltwin 2025
# =============================================================================
import os
import cv2
import imutils

ACCEPTED_FORMAT = [
    "png",
    "jpeg",
    "jpg"
]
OUTPUT_EXTENSION = "hpp"

HPP_TEMPLATE = """#ifndef {0}_HPP
#define {0}_HPP

#include "kami/utils/img.hpp"

constexpr uint8_t {0}_RAW[] = {{ {2} }};
const KamiImage {1}({0}_RAW, {3}, {4});

#endif
"""


def format_name(name: str, header: bool):
    if header:
        return name.upper()
    return name.replace("_", " ").title().replace(" ", "")


MAX_WIDTH = 256


def compile_image(input_path: str, output_path: str, file_name: str) -> None:
    print(f"Found {input_path} -> {output_path}")
    header_name = format_name(file_name, True)
    class_name = format_name(file_name, False)

    # Read input image
    img = cv2.imread(input_path)
    h, w, c = img.shape

    # Resize image if too big
    max_dim = (0, h) if h > w else (1, w)
    if max_dim[1] > MAX_WIDTH:
        if max_dim[0] == 0:
            img = imutils.resize(img, height=MAX_WIDTH)
        else:
            img = imutils.resize(img, width=MAX_WIDTH)
        h, w, c = img.shape

    raw_data = ""
    for row_idx in range(h):
        for col_idx in range(w):
            for channel_idx in range(c):
                raw_data += str(img[row_idx, col_idx, channel_idx]) + (
                    "," if not (row_idx == h - 1 and col_idx == w - 1 and channel_idx != c - 1) else "")

    # Create directory if it does not exist
    res_dir = os.path.dirname(output_path)
    if not os.path.exists(res_dir):
        os.makedirs(res_dir)

    # Export the HPP file
    with open(output_path, "w+") as hpp_file:
        hpp_file.write(HPP_TEMPLATE.format(
            header_name,
            class_name,
            raw_data,
            w,
            h
        ))


if __name__ == "__main__":
    import argparse as ag

    parser = ag.ArgumentParser()
    parser.add_argument("res_path", type=str)
    parser.add_argument("-b", "--build", type=str, default="build/im2cpp/kami/res")
    args = parser.parse_args()

    for dirpath, _, files in os.walk(args.res_path):
        for f in files:
            f_exploded = f.split(".")
            ext = f_exploded[-1] if len(f_exploded) > 1 else ""
            if ext in ACCEPTED_FORMAT:
                compile_image(os.path.join(dirpath, f),  # type:ignore
                              os.path.join(dirpath, f)  # type:ignore
                              .replace(args.res_path, args.build)
                              .replace(ext, OUTPUT_EXTENSION),
                              ".".join(f_exploded[:-1]))
