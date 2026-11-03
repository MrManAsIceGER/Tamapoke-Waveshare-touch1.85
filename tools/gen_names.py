#!/usr/bin/env python3
"""Fetch official localized names from PokeAPI and write tools/dex_names.py.

  python3 tools/gen_names.py

Only FR and DE have distinct Gen 1 names; ES/IT/PT use the English names,
so this file stores only entries that differ from English. The GFX font is
ASCII, so Latin-script names are written in uppercase without accents.
"""
import json
import os
import subprocess
import time
import unicodedata

LANGS = ('fr', 'de')
LANGS_UTF8 = ('ja-hrkt', 'ko')  # CJK: keep UTF-8 as-is; do not convert it to ASCII octal escapes

# Gender-label convention by language: neither the CP437 table nor any U8g2
# Unifont subset includes ♀/♂, so each language chooses how to represent them.
# The Latin path has always used F/M. Korean uses 암/수
# ('female'/'male'), read as words rather than single letters, and
# they are SEPARATED BY A SPACE: 니드런 암. The space belongs in the replacement
# because only the symbol itself is replaced. Both syllables are in
# KS X 1001 and already appear in the table (암나이트, 암스타, 홍수몬). The name has
# four syllables plus a space, compared with five-syllable entries such as 깨비드릴조,
# so it is not the widest entry in the table.
GENERO = {'ko': (' 암', ' 수')}
GENERO_DEF = ('F', 'M')


def sin_genero(s, lang=None):
    """Nidoran hembra/macho: ♀ y ♂ no existen en ninguna fuente que usemos.

    Ni la tabla CP437 de glcdfont.h los tiene en una posicion imprimible, ni
    los trae ningun subconjunto unifont de U8g2 (japanese*, korean*, chinese*).
    Y Arduino_GFX no dibuja nada cuando le falta el glifo, ni siquiera avanza
    el cursor, asi que sin esto los dos Nidoran salen con el mismo nombre.
    """
    hembra, macho = GENERO.get(lang, GENERO_DEF)
    return s.replace('♀', hembra).replace('♂', macho)


def ascii_up(s):
    """'Salamèche' -> 'SALAMECHE'; conserva los simbolos de genero como f/m."""
    s = sin_genero(s)
    s = unicodedata.normalize('NFD', s)
    s = ''.join(c for c in s if unicodedata.category(c) != 'Mn')
    return s.upper()


def fetch(num):
    url = f'https://pokeapi.co/api/v2/pokemon-species/{num}/'
    for intento in range(4):
        r = subprocess.run(['curl', '-s', '--max-time', '25', '-A', 'Mozilla/5.0', url],
                           capture_output=True, text=True)
        if r.returncode == 0 and r.stdout.strip().startswith('{'):
            return json.loads(r.stdout)
        time.sleep(1 + intento)
    raise SystemExit(f'no se pudo bajar la especie {num}')


def main():
    out = {}
    for num in range(1, 152):
        d = fetch(num)
        names = {n['language']['name']: n['name'] for n in d['names']}
        en = ascii_up(names['en'])
        dif = {}
        for lg in LANGS:
            v = ascii_up(names.get(lg, names['en']))
            if v != en:
                dif[lg] = v
        for lg in LANGS_UTF8:
            v = names.get(lg)
            if v:
                # 'ja-hrkt' -> 'ja'; preserve UTF-8 except for ♀/♂, which the
                # font does not contain (see no_gender)
                corto = lg.split('-')[0]
                dif[corto] = sin_genero(v, corto)
        if dif:
            out[num] = dif
        if num % 25 == 0:
            print(f'  {num}/151...')
        time.sleep(0.05)

    path = os.path.join(os.path.dirname(__file__), 'dex_names.py')
    with open(path, 'w', encoding='utf-8') as f:
        f.write('# -*- coding: utf-8 -*-\n')
        f.write('"""GENERADO por tools/gen_names.py desde PokeAPI - no editar a mano.\n\n')
        f.write('Nombres oficiales por idioma. FR y DE van en mayusculas y sin\n')
        f.write('acentos (fuente CP437, un byte por caracter); JA y KO van en\n')
        f.write('UTF-8 tal cual, porque se pintan con una fuente U8g2.\n"""\n\n')
        f.write('LOCAL_NAMES = {\n')
        for num in sorted(out):
            f.write(f'    {num}: {out[num]!r},\n')
        f.write('}\n')
    print(f'guardado {os.path.normpath(path)}: {len(out)} especies con nombre propio')


if __name__ == '__main__':
    main()
