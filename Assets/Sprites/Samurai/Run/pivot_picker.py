import tkinter as tk
from PIL import Image, ImageTk
import os
import json

# CONFIGURAÇÕES
sprite_folder = "/Users/gustavo/CLionProjects/TPFINAL/Assets/Sprites/Samurai/Run"  # pasta com as imagens
output_json = "pivots.json"        # onde salvar o resultado
sprite_size = (64, 64)             # tamanho das sprites (ajuste se necessário)
zoom_factor = 4                    # fator de zoom

# JSON onde vamos guardar os pivots
pivot_data = {}

# Lista de arquivos
sprite_files = sorted([
    f for f in os.listdir(sprite_folder)
    if f.endswith(".png")
])
current_index = 0

# Função para salvar o clique como pivot
def on_click(event):
    global current_index

    img_width, img_height = sprite_size

    # Coordenadas absolutas em pixels (corrigidas pelo zoom)
    pixel_x = event.x // zoom_factor
    pixel_y = event.y // zoom_factor

    # Coordenadas relativas entre 0 e 1
    rel_x = round(pixel_x / img_width, 4)
    rel_y = round(pixel_y / img_height, 4)

    filename = sprite_files[current_index]
    pivot_data[filename] = {
        "pivot_pixel": {"x": pixel_x, "y": pixel_y},
        "pivot_relativo": {"x": rel_x, "y": rel_y}
    }

    print(f"{filename} → pixel: ({pixel_x}, {pixel_y}), relativo: ({rel_x}, {rel_y})")

    current_index += 1
    if current_index < len(sprite_files):
        load_image()
    else:
        with open(output_json, "w") as f:
            json.dump(pivot_data, f, indent=4)
        print("✅ Todos os pivots salvos em", output_json)
        root.destroy()


# Função para carregar a próxima imagem
def load_image():
    img = Image.open(os.path.join(sprite_folder, sprite_files[current_index]))
    zoomed_img = img.resize(
        (img.width * zoom_factor, img.height * zoom_factor),
        Image.NEAREST  # mantém visual pixelado
    )
    tk_img = ImageTk.PhotoImage(zoomed_img)
    canvas.itemconfig(img_id, image=tk_img)
    canvas.image = tk_img
    root.title(f"Selecionando pivot para: {sprite_files[current_index]}")

# GUI setup
root = tk.Tk()
canvas = tk.Canvas(root, width=sprite_size[0]*zoom_factor, height=sprite_size[1]*zoom_factor)
canvas.pack()
img_id = canvas.create_image(0, 0, anchor="nw")
canvas.bind("<Button-1>", on_click)

load_image()
root.mainloop()
