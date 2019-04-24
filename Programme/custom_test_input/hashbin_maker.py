# -*- coding: utf-8 -*-

import hashlib
import string
import random


def create_bin(in_path, out_path):
    # A partir d'un fichier avec un mot par ligne, on cree un fichier de hashes sha256
    # ecrit en binaire et ou les hash sont concatenes
    with open(out_path, "wb") as out_file:
        with open(in_path, "r") as in_file:
            line = in_file.readline()
            while line != "":
                out_file.write(hashlib.sha256(line[:-1].encode()).digest())
                line = in_file.readline()

def create_txt(out_path, n_pwd, pwd_len=4):
    # Cree un fichier contenant n_pwd pwd de pwd_len caracteres (un pwd par ligne)
    # On utilise que des lettres minuscules
    with open(out_path, "w") as out_file:
        for i in range(n_pwd):
            pwd = "".join([random.choice(string.ascii_lowercase) for _ in range(pwd_len)])
            out_file.write(pwd + "\n")


if __name__ == "__main__":
    create_txt("./custom_hashes_1.txt", 10, 4)
    create_bin("./custom_hashes_1.txt", "./custom_hashes_1.bin")
