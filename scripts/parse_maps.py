import cv2
import math
from tqdm import tqdm

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d


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

water_color_map = {
    (0, 0, 0): 0,
    (0, 238, 255): 1
}

resource_color_map = {
    (255, 255, 255): 0,  # Nothing
    (51, 51, 51): 1,  # Best coal
    (92, 92, 92): 2,  # Medium coal
    (138, 138, 138): 3,  # Low coal
    (238, 237, 0): 4,  # Gold
    (168, 156, 103): 5,  # Iron
}

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


def process_maps():
    elev_img = cv2.cvtColor(cv2.imread("img/usa_topo_iso.png"), cv2.COLOR_BGR2RGB)
    water_img = cv2.cvtColor(cv2.imread("img/usa_water_iso.png"), cv2.COLOR_BGR2RGB)
    temp_img = cv2.cvtColor(cv2.imread("img/usa_temp_iso.png"), cv2.COLOR_BGR2RGB)
    precip_img = cv2.cvtColor(cv2.imread("img/usa_precip_iso.png"), cv2.COLOR_BGR2RGB)
    resource_img = cv2.cvtColor(cv2.imread("img/usa_resource_iso.png"), cv2.COLOR_BGR2RGB)
    biome_img = cv2.cvtColor(cv2.imread("img/usa_biome_iso.png"), cv2.COLOR_BGR2RGB)

    features = 7

    feature_matrix = np.zeros((elev_img.shape[1], elev_img.shape[0], features))

    pbar = tqdm(total=elev_img.shape[0]*elev_img.shape[1])
    progress = 0
    for y in range(elev_img.shape[0]):
        for x in range(elev_img.shape[1]):
            elevation = list(topo_color_map.values())[math.ceil(elev_img[y, x][0]/5.5)]
            if elevation is None:
                raise ValueError(f"Color {elev_img[y, x]} not found in elevation color map")

            temp_score = list(temp_color_map.values())[temp_img[y, x][0]//15]
            if temp_score is None:
                raise ValueError(f"Color {temp_img[y, x]} not found in temp color map")

            precip_score = list(precip_color_map.values())[precip_img[y, x][0]//15]
            if precip_score is None:
                raise ValueError(f"Color {precip_img[y, x]} not found in precip color map")

            water_score = water_color_map[tuple(water_img[y, x])]
            if water_score is None:
                raise ValueError(f"Color {water_img[y, x]} not found in water color map")

            resource_type = resource_color_map[tuple(resource_img[y, x])]
            if resource_type is None:
                raise ValueError(f"Color {resource_img[y, x]} not found in resource color map")

            biome_type = biome_color_map[tuple(biome_img[y, x])]
            if biome_type is None:
                raise ValueError(f"Color {biome_img[y, x]} not found in biome color map")

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

            pbar.update(1)
            progress += 1

    pbar.close()

    return feature_matrix


if __name__ == "__main__":

    feature_matrix = process_maps()

    feature_matrix.astype(np.ushort).tofile("out/initial_state.npy")

    # select_features = feature_matrix[:, :, 0]
    # X, Y = np.meshgrid(np.arange(select_features.shape[1]), np.arange(select_features.shape[0]))
    #
    # fig = plt.figure()
    # ax = fig.add_subplot(111, projection='3d')
    # ax.plot_surface(X, Y, select_features)
    #
    # ax.set_xlabel('X')
    # ax.set_ylabel('Y')
    # ax.set_zlabel('Z')
    # ax.set_title('3D Surface Plot')
    # ax.view_init(elev=30, azim=-45)
    #
    # plt.show()

    print(feature_matrix.shape)
    print(feature_matrix[0, 0])
