import cv2
import math
from tqdm import tqdm

import numpy as np
import argparse


def reverse_map(to_reverse: dict):
    return {v: k for k, v in to_reverse.items()}


# Elevation in feet
topo_color_map = {
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
topo_color_map_rev = reverse_map(topo_color_map)

# Average yearly temperature in Fahrenheit
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
temp_color_map_rev = reverse_map(temp_color_map)

# Inches of precipitation per year
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
precip_color_map_rev = reverse_map(precip_color_map)

# Trade usefulness of water
water_color_map = {
    (0, 0, 0): 0,  # No water
    (0, 238, 255): 1,  # Low-trade Water
    (0, 192, 255): 2,  # Medium-trade Water
    (0, 158, 255): 3,  # High-trade Water
}
water_color_map_rev = reverse_map(water_color_map)

# Distribution of industrial/precious resources
resource_color_map = {
    (255, 255, 255): 0,  # Nothing
    (51, 51, 51): 1,  # Best coal
    (92, 92, 92): 2,  # Medium coal
    (138, 138, 138): 3,  # Low coal
    (238, 237, 0): 4,  # Gold
    (168, 156, 103): 5,  # Iron
}
resource_color_map_rev = reverse_map(resource_color_map)

# Biome type
biome_color_map = {
    (0, 0, 0): 0,  # Ocean
    (121, 165, 88): 1,  # Conifer Forest
    (130, 186, 147): 2,  # Deciduous Forest
    (176, 205, 175): 3,  # Grassland
    (177, 207, 153): 4,  # Marshland
    (255, 230, 193): 5,  # Desert
    (231, 219, 161): 6,  # Shrubland
    (11, 139, 73): 7,  # Tropical Forest
}
biome_color_map_rev = reverse_map(biome_color_map)

# Population per square mile
# NOTE: Each cell is about 9 square miles, so multiply by 9 to get population per cell
cities_color_map = {
    (0, 0, 0): 0,
    (0, 255, 98): 10,
    (53, 244, 0): 100,
    (168, 244, 0): 500,
    (219, 244, 0): 1000,
    (244, 237, 0): 2500,
    (244, 211, 0): 5000,
    (244, 179, 0): 7500,
    (244, 134, 0): 10000,
    (255, 100, 0): 15000,
    (255, 70, 0): 20000,
    (255, 43, 0): 25000,
    (255, 54, 50): 30000,
    (255, 123, 119): 40000,
    (255, 184, 183): 50000,
    (255, 255, 255): 60000,
}

cities_color_map_rev = reverse_map(cities_color_map)

def get_best_population(population: int):

    pop_values = list(cities_color_map.values())

    if population == pop_values[0]:
        return cities_color_map_rev[pop_values[1]]
    if population >= pop_values[-1]:
        return cities_color_map_rev[pop_values[-1]]

    for i in range(len(pop_values)-1):

        if pop_values[i] <= population < pop_values[i+1]:
            # Give whichever is closest
            if population - pop_values[i] < pop_values[i+1] - population:
                return cities_color_map_rev[pop_values[i]]
            else:
                return cities_color_map_rev[pop_values[i+1]]

    raise ValueError(f"Population {population} not found in population color map")


# Number of features per cell
CELL_FEATURES = 8


def import_array(image_data: np.ndarray, out_file: str):
    output_img = np.zeros((image_data.shape[1], image_data.shape[0], 4), np.uint8)

    pbar = tqdm(total=image_data.shape[1] * image_data.shape[0])
    progress = 0
    for y in range(image_data.shape[1]):
        for x in range(image_data.shape[0]):

            alpha = 255 if image_data[x, y][7] != 0 else 0
            output_img[y, x] = np.asarray(list(get_best_population(image_data[x, y][7])[::-1]) + [alpha])

            pbar.update(1)
            progress += 1

    pbar.close()

    cv2.imwrite(out_file, output_img)


def export_array(city_map: str):
    elev_img = cv2.cvtColor(cv2.imread("img/usa_topo_iso.png"), cv2.COLOR_BGR2RGB)
    water_img = cv2.cvtColor(cv2.imread("img/usa_water_iso.png"), cv2.COLOR_BGR2RGB)
    temp_img = cv2.cvtColor(cv2.imread("img/usa_temp_iso.png"), cv2.COLOR_BGR2RGB)
    precip_img = cv2.cvtColor(cv2.imread("img/usa_precip_iso.png"), cv2.COLOR_BGR2RGB)
    resource_img = cv2.cvtColor(cv2.imread("img/usa_resource_iso.png"), cv2.COLOR_BGR2RGB)
    biome_img = cv2.cvtColor(cv2.imread("img/usa_biome_iso.png"), cv2.COLOR_BGR2RGB)
    city_img = cv2.cvtColor(cv2.imread(city_map), cv2.COLOR_BGR2RGB)

    feature_matrix = np.zeros((elev_img.shape[1], elev_img.shape[0], CELL_FEATURES))

    pbar = tqdm(total=elev_img.shape[0]*elev_img.shape[1])
    progress = 0
    for y in range(elev_img.shape[0]):
        for x in range(elev_img.shape[1]):
            elevation = list(topo_color_map.values())[math.ceil(elev_img[y, x][0]/5.5)]
            if elevation is None:
                raise ValueError(f"Color {elev_img[y, x]} not found in elevation color map @ ({x}, {y})")

            temp_score = list(temp_color_map.values())[temp_img[y, x][0]//15]
            if temp_score is None:
                raise ValueError(f"Color {temp_img[y, x]} not found in temp color map @ ({x}, {y})")

            precip_score = list(precip_color_map.values())[precip_img[y, x][0]//15]
            if precip_score is None:
                raise ValueError(f"Color {precip_img[y, x]} not found in precip color map @ ({x}, {y})")

            water_score = water_color_map.get(tuple(water_img[y, x]))
            if water_score is None:
                raise ValueError(f"Color {water_img[y, x]} not found in water color map @ ({x}, {y})")

            resource_type = resource_color_map.get(tuple(resource_img[y, x]))
            if resource_type is None:
                raise ValueError(f"Color {resource_img[y, x]} not found in resource color map @ ({x}, {y})")

            biome_type = biome_color_map.get(tuple(biome_img[y, x]))
            if biome_type is None:
                raise ValueError(f"Color {biome_img[y, x]} not found in biome color map @ ({x}, {y})")

            population = cities_color_map.get(tuple(city_img[y, x]))
            if population is None:
                raise ValueError(f"Color {city_img[y, x]} not found in city color map @ ({x}, {y})")

            gradient = 0
            if 0 < y < elev_img.shape[0] - 1 and 0 < x < elev_img.shape[1] - 1:
                y_diff = list(topo_color_map.values())[int(elev_img[y-1, x][0]/5.5)] - list(topo_color_map.values())[int(elev_img[y+1, x][0]/5.5)]
                x_diff = list(topo_color_map.values())[int(elev_img[y, x-1][0]/5.5)] - list(topo_color_map.values())[int(elev_img[y, x+1][0]/5.5)]

                scored_diff = max(abs(y_diff), abs(x_diff))
                feet_per_pixel = 15632
                gradient = scored_diff / feet_per_pixel

            feature_matrix[x, y][0] = elevation
            feature_matrix[x, y][1] = gradient
            feature_matrix[x, y][2] = water_score
            feature_matrix[x, y][3] = temp_score
            feature_matrix[x, y][4] = precip_score
            feature_matrix[x, y][5] = resource_type
            feature_matrix[x, y][6] = biome_type
            feature_matrix[x, y][7] = population

            pbar.update(1)
            progress += 1

    pbar.close()

    return feature_matrix


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("action", choices=["import", "export"], help="Whether to import an array to make an image or export an image into an array")
    parser.add_argument("-i", "--input", help="The array or city map to import")
    parser.add_argument("-o", "--output", help="The array or image to export")
    args = parser.parse_args()

    if args.action == "import":
        if args.input is None:
            raise ValueError("Must provide input array to import")
        if args.output is None:
            raise ValueError("Must provide output image to create")

        image_data = np.fromfile(args.input, dtype=np.ushort).reshape((994, 623, CELL_FEATURES))
        import_array(image_data, args.output)

    elif args.action == "export":
        if args.input is None:
            raise ValueError("Must provide input city map to export")
        if args.output is None:
            raise ValueError("Must provide output array to create")

        feature_matrix = export_array(args.input)
        feature_matrix.astype(np.ushort).tofile(args.output)


if __name__ == "__main__":

    main()
