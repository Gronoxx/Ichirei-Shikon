import os
import re
import json
from tkinter import Tk, Canvas
from PIL import Image, ImageTk

def select_pivot(image_path):
    img = Image.open(image_path)
    zoom = 4
    img_zoomed = img.resize((img.width * zoom, img.height * zoom), Image.NEAREST)

    root = Tk()
    root.title(f"Selecione o pivô para: {os.path.basename(image_path)}")
    canvas = Canvas(root, width=img_zoomed.width, height=img_zoomed.height)
    canvas.pack()

    tk_img = ImageTk.PhotoImage(img_zoomed)
    canvas.create_image(0, 0, anchor="nw", image=tk_img)

    pivot_coords = []

    def on_click(event):
        x = event.x / zoom
        y = event.y / zoom
        pivot_coords.append((x, y))
        root.destroy()

    canvas.bind("<Button-1>", on_click)
    root.mainloop()

    return pivot_coords[0] if pivot_coords else (0.0, 0.0)

def sanitize_folder_name(folder_name):
    folder_name = re.sub(r'\W+', '_', folder_name)
    return folder_name

# Caminho base passado como argumento
import sys
if len(sys.argv) != 2:
    print("Uso: python3 transitions_pivot.py <caminho>")
    exit(1)

base_path = sys.argv[1]
result = {}

for folder in sorted(os.listdir(base_path)):
    folder_path = os.path.join(base_path, folder)
    if not os.path.isdir(folder_path):
        continue

    pngs = [
        f for f in os.listdir(folder_path)
        if f.lower().endswith(".png") and re.match(rf"^{re.escape(folder)}(\d+)\.png$", f)
    ]

    if not pngs:
        print(f"⚠️ Nenhum frame válido encontrado em '{folder}'")
        continue

    def extract_number(filename):
        match = re.search(rf"{re.escape(folder)}(\d+)\.png", filename)
        return int(match.group(1)) if match else -1

    pngs.sort(key=extract_number)
    first_png = pngs[0]
    last_png = pngs[-1]

    first_path = os.path.join(folder_path, first_png)
    last_path = os.path.join(folder_path, last_png)

    print(f"\n📂 Processando '{folder}' → {first_png}, {last_png}")

    pivot1 = select_pivot(first_path)
    pivot2 = select_pivot(last_path)

    clean_name = sanitize_folder_name(folder)
    result[clean_name] = {
        "transitionPivots": [
            {"x": round(pivot1[0], 1), "y": round(pivot1[1], 1)},
            {"x": round(pivot2[0], 1), "y": round(pivot2[1], 1)}
        ]
    }

# 🔧 Salva o JSON dentro da pasta base fornecida
output_path = os.path.join(base_path, "transitions_pivots.json")
with open(output_path, "w") as f:
    json.dump(result, f, indent=2)

print(f"\n✅ Arquivo salvo em: {output_path}")
