Szeretnénk egy GUI rendszert, ami:
- nagyjából értelmesen van felépítve
- a use-case-k nagyrésze kifejezhető vele
- valahogy regulálva van az információ áramlása és az update-k egymásutánisága, hogy ne legyen katyvasz
- szeretném, ha azonosítani lehetne az egyes GUI elemeket azután is, hogy bele raktuk őket a rendszerbe
- szeretném, ha ezek tudnának hivatkozni egymásra, és tudnának egymásnak is üzenni

- jó lenne csökkenteni az információ propagálásában a közvetítő lépéseket

Az információ áramlás rendezése céljából szétválasztjuk, hogy mikor változhat egy widget belső állapota.

A widgetek belső állapota külső események (egér, bill event) hatására, vagy másik widgetek hatására változhat meg.
Lenne egy dedikált alulról felfelé bejárás, amikor ez a változás megtörténthet.
