import json
import re

with open("texture.json") as f:
    texture_data = json.load(f)

# Função para extrair o número do nome do arquivo RunX.png
def extract_frame_number(filename):
    # Exemplo: Run10.png → 10
    match = re.search(r'Run(\d+)\.png', filename)
    if match:
        return int(match.group(1))
    return -1  # ou algum valor default para erro

# Gera o vetor dos pivots em pixel (como antes)
with open("offsets.cpp", "w") as out_file:
    out_file.write("// Vetor com os offsets de pivô\n")
    out_file.write("std::vector<Vector2> runOffsets = {\n")
    for frame in texture_data["frames"]:
        filename = frame["filename"]
        # Aqui você coloca seu método de pegar pivot, por exemplo dummy (0,0)
        out_file.write(f"    Vector2(0, 0), // {filename}\n")
    out_file.write("};\n\n")

    # Gera o vetor com a ordem da animação pelo número do frame no nome, ajustando para índice zero
    out_file.write("// Ordem da animação, número do frame extraído do nome do arquivo (zero-based)\n")
    out_file.write("std::vector<int> runAnimationOrder = {\n")
    for frame in texture_data["frames"]:
        filename = frame["filename"]
        number = extract_frame_number(filename)
        if number >= 0:
            number -= 1  # ajusta para índice zero
        out_file.write(f"    {number}, // {filename}\n")
    out_file.write("};\n")


print("Arquivo offsets.cpp com os dois vetores gerado com sucesso!")
