import math
from enum import Enum

import cv2
import numpy as np

from tqdm import tqdm

topo_color_map = {
    # (0, 0, 0): 0,
    # (64, 115, 116): 0,
    (127, 255, 255): 0,
    (255, 227, 227): 25,
    (255, 209, 209): 50,
    (255, 182, 182): 75,
    (255, 155, 155): 100,
    (255, 136, 136): 150,
    (255, 118, 118): 200,
    (255, 91, 91): 250,
    (255, 72, 72): 300,
    (255, 45, 45): 350,
    (255, 18, 18): 400,
    (255, 0, 0): 450,
    (237, 0, 0): 500,
    (208, 0, 0): 600,
    (179, 0, 0): 700,
    (151, 0, 0): 800,
    (130, 5, 0): 900,
    (145, 34, 0): 1100,
    (159, 63, 0): 1200,
    (177, 98, 0): 1400,
    (191, 127, 0): 1600,
    (205, 156, 0): 1800,
    (220, 185, 0): 2000,
    (234, 214, 0): 2200,
    (249, 243, 0): 2400,
    (223, 244, 0): 2600,
    (170, 226, 0): 2800,
    (106, 205, 0): 3100,
    (53, 187, 0): 3400,
    (0, 170, 0): 3800,
    (0, 131, 45): 4200,
    (0, 92, 90): 4600,
    (0, 54, 136): 5000,
    (0, 15, 181): 5400,
    (15, 0, 190): 5800,
    (46, 0, 171): 6200,
    (72, 0, 156): 6600,
    (98, 0, 140): 7200,
    (125, 0, 125): 7800,
    (140, 30, 140): 8600,
    (155, 60, 155): 9400,
    (171, 91, 171): 10200,
    (173, 95, 173): 11000,
    (175, 98, 174): 11800,
    (177, 101, 176): 12600,
    (180, 105, 178): 13400,
    (180, 108, 179): 14200,
}

temp_color_map = {
    (5, 51, 106): 20,
    (0, 74, 133): 24,
    (40, 97, 160): 28,
    (29, 127, 187): 31,
    (8, 155, 214): 35,
    (95, 181, 222): 39,
    (172, 203, 228): 42,
    (214, 227, 239): 46,
    (255, 255, 255): 50,
    (255, 219, 146): 54,
    (252, 187, 109): 58,
    (255, 133, 93): 61,
    (237, 96, 76): 65,
    (216, 57, 56): 69,
    (184, 26, 67): 72,
    (135, 27, 65): 76,
    (106, 2, 44): 80
}

precip_color_map = {
    (229, 235, 223): 0,
    (235, 247, 229): 5,
    (223, 243, 218): 10,
    (214, 239, 208): 15,
    (204, 235, 197): 20,
    (186, 228, 189): 25,
    (168, 221, 181): 30,
    (145, 212, 188): 35,
    (122, 203, 196): 40,
    (99, 191, 204): 45,
    (79, 179, 211): 50,
    (60, 159, 200): 55,
    (43, 140, 190): 60,
    (26, 122, 181): 65,
    (9, 105, 173): 70,
    (8, 85, 151): 75,
    (8, 66, 131): 80
}


class ProcessEnum(Enum):
    ELEVATION = 0
    COLOR_ELEV = 1
    GRADIENT = 2
    TEMP = 3
    PRECIP = 4


def color_distance(color1: tuple[int, int, int], color2: tuple[int, int, int]):
    rmean = (color1[0] + color2[0]) // 2
    r = color1[0] - color2[0]
    g = color1[1] - color2[1]
    b = color1[2] - color2[2]
    return math.sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8))


def extract_isoline(iso_color_map, color: tuple[int, int, int]):

    min_dist = 256*3
    best_isoline = 0
    color_idx = 0
    best_color = (255, 255, 255)
    for i, (iso_color, iso_value) in enumerate(iso_color_map.items()):
        dist = color_distance(color, iso_color)
        if dist < min_dist:
            min_dist = dist
            best_isoline = iso_value
            best_color = iso_color
            color_idx = i

    return color_idx, best_color, best_isoline


def process_image(processType: ProcessEnum = ProcessEnum.ELEVATION):
    if processType in [ProcessEnum.ELEVATION, ProcessEnum.COLOR_ELEV, ProcessEnum.GRADIENT]:
        in_name, out_name, color_map = "usa_topo.png", "usa_topo_iso.png", topo_color_map
    elif processType == ProcessEnum.TEMP:
        in_name, out_name, color_map = "usa_temp.png", "usa_temp_iso.png", temp_color_map
    elif processType == ProcessEnum.PRECIP:
        in_name, out_name, color_map = "usa_precip.png", "usa_precip_iso.png", precip_color_map
    else:
        raise ValueError(f"Invalid process type: {processType}")

    raw_img = cv2.cvtColor(cv2.imread(in_name), cv2.COLOR_BGR2RGB)
    output_img = raw_img.copy()

    pbar = tqdm(total=output_img.shape[0]*output_img.shape[1])
    progress = 0
    for y in range(output_img.shape[0]):
        for x in range(output_img.shape[1]):
            color_idx, best_color, best_isoline = extract_isoline(color_map, raw_img[y, x])
            color_vals = [0, 0, 0]
            match processType:
                case ProcessEnum.COLOR_ELEV:
                    color_vals = best_color
                case ProcessEnum.GRADIENT:
                    gradient = 0
                    if 0 < y < output_img.shape[0] - 1 and 0 < x < output_img.shape[1] - 1:
                        gradient = min(math.log(abs((extract_isoline(color_map, raw_img[y - 1, x])[1] - extract_isoline(color_map, raw_img[y + 1, x])[1])) + 1) / 12 * 255, 255)
                    color_vals = [gradient, gradient, gradient]
                case ProcessEnum.ELEVATION:
                    elevation_color = int(color_idx * 5.5)
                    color_vals = [elevation_color, elevation_color, elevation_color]
                case ProcessEnum.TEMP:
                    temp_color = int(color_idx * 15)
                    color_vals = [temp_color, temp_color, temp_color]
                case ProcessEnum.PRECIP:
                    precip_color = int(color_idx * 15)
                    color_vals = [precip_color, precip_color, precip_color]

            output_img[y, x] = np.array(color_vals)[::-1]

            pbar.update(1)
            progress += 1

    pbar.close()

    cv2.imwrite(out_name, output_img)
    cv2.destroyAllWindows()


if __name__ == "__main__":

    process_image(ProcessEnum.ELEVATION)

"""    grey_map = {}
    for i, (color, elev) in enumerate(color_map.items()):
        grey_color = int(i * 5.5)

        if grey_color in grey_map:
            raise ValueError(f"Color {grey_color} already in map")

        grey_map[grey_color] = elev

    print(grey_map)"""


