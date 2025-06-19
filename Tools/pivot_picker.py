import tkinter as tk
from PIL import Image, ImageTk
import os
import json
import sys

# ===== VERIFICAÇÃO DE ARGUMENTOS =====
if len(sys.argv) < 2:
    print("❌ Uso: python nome_do_arquivo.py /caminho/para/imagens")
    sys.exit(1)

# Diretório passado como argumento
sprite_folder = sys.argv[1]

# Verifica se o diretório existe
if not os.path.isdir(sprite_folder):
    print(f"❌ Diretório inválido: {sprite_folder}")
    sys.exit(1)

# Nome do JSON de saída dentro do mesmo diretório
output_json = os.path.join(sprite_folder, "pivots.json")

# Configurações fixas
sprite_size = (64, 64)
zoom_factor = 4

# JSON onde vamos guardar os pivots
pivot_data = {}

# Lista de arquivos
sprite_files = sorted([
    f for f in os.listdir(sprite_folder)
    if f.endswith(".png")
])
current_index = 0

# ===== FUNÇÕES PRINCIPAIS =====

def on_click(event):
    global current_index

    img_width, img_height = sprite_size

    pixel_x = event.x // zoom_factor
    pixel_y = event.y // zoom_factor

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

def load_image():
    img = Image.open(os.path.join(sprite_folder, sprite_files[current_index]))
    zoomed_img = img.resize(
        (img.width * zoom_factor, img.height * zoom_factor),
        Image.NEAREST
    )
    tk_img = ImageTk.PhotoImage(zoomed_img)
    canvas.itemconfig(img_id, image=tk_img)
    canvas.image = tk_img
    root.title(f"Selecionando pivot para: {sprite_files[current_index]}")

# ===== GUI SETUP =====
root = tk.Tk()
canvas = tk.Canvas(root, width=sprite_size[0]*zoom_factor, height=sprite_size[1]*zoom_factor)
canvas.pack()
img_id = canvas.create_image(0, 0, anchor="nw")
canvas.bind("<Button-1>", on_click)

load_image()
root.mainloop()
