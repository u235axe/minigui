Szeretnénk egy GUI rendszert, ami:
- nagyjából értelmesen van felépítve
- a use-case-k nagyrésze kifejezhető vele
- valahogy regulálva van az információ áramlása és az update-k egymásutánisága, hogy ne legyen katyvasz
- szeretném, ha egyedileg azonosítani lehetne az egyes GUI elemeket azután is, hogy bele raktuk őket a rendszerbe
- szeretném, ha ezek tudnának hivatkozni egymásra, és tudnának egymásnak is üzenni
- jó lenne csökkenteni az információ propagálásában a közvetítő lépéseket

Az információ áramlás rendezése céljából szétválasztjuk, hogy mikor változhat egy widget belső állapota.

* A widgetek belső állapota külső események (egér, bill event) hatására, vagy másik widgetek hatására változhat meg.
* Lenne egy dedikált alulról felfelé bejárás, amikor ez a változás megtörténhet.
* Valószínűleg ugyanekkor a bejárás során a widget visszaadja az eseményre adott megváltozásának információit (pl. megváltozott egy textbox tartalma), ez felfelé propagálódik és a parentek csinálhatnak vele valamit, vagy legkívül az end-user.

Egy másik bejárásban az alignment kiértékelése történik meg
* A widget meg tudja magától mondani, hogy mi a preferált mérete, ez az információ felfelé propagálódik 
* A containerek a widgetek méreteinek ismeretében kiszámolhatják a saját méreteiket, illetve elhelyezhetik a gyerekeiket (ez az elhelyezés a méretet már nem változtatja meg)

* Léteznek kényszerek (irányított?) megszorítások a méretezésekre, és elhelyezésekre, amik widgetek között vannak (ezt lehet, hogy meg kell szorítani, de jó lenne, ha nem kellene).
* A kényszerek az egyik widget valamilyen méret paramétereének függvényében adnak megszorítást egy másik widget méretparaméterére.
* Kérdés: Mi van, ha ellentmondanak a kényszerek?
* Valaki, valahol a kényszerek figyelembevételével végzi el a méretezést és az alignmentet

