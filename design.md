Szeretnénk egy GUI rendszert, ami:
- nagyjából értelmesen van felépítve
- a use-case-k nagyrésze kifejezhető vele
- valahogy regulálva van az információ áramlása és az update-k egymásutánisága, hogy ne legyen katyvasz
- szeretném, ha egyedileg azonosítani lehetne az egyes GUI elemeket azután is, hogy bele raktuk őket a rendszerbe
- szeretném, ha ezek tudnának hivatkozni egymásra, és tudnának egymásnak is üzenni
- jó lenne csökkenteni az információ propagálásában a közvetítő lépéseket
- többek között ki akarjuk zárni a végtelen ciklikus updateket widgetek között

Az információ áramlás rendezése céljából szétválasztjuk, hogy mikor változhat egy widget belső állapota.

* A widgetek belső állapota külső események (egér, bill event) hatására, vagy másik widgetek hatására változhat meg.
* Lenne egy dedikált alulról felfelé bejárás, amikor ez a változás megtörténhet.
* Valószínűleg ugyanekkor a bejárás során a widget visszaadja az eseményre adott megváltozásának információit (pl. megváltozott egy textbox tartalma), ez felfelé propagálódik és a parentek csinálhatnak vele valamit, vagy legkívül az end-user.
* A widgetek kéne, hogy lássák az összes hozzájuk (alulról?) érkező eseményt, mielőtt elkezdenek reagálni rájuk.

Egy másik bejárásban az alignment kiértékelése történik meg
* A widget meg tudja magától mondani, hogy mi a preferált mérete, ez az információ felfelé propagálódik 
* A containerek a widgetek méreteinek ismeretében kiszámolhatják a saját méreteiket, illetve elhelyezhetik a gyerekeiket (ez az elhelyezés a méretet már nem változtatja meg)

* Léteznek kényszerek (irányított?) megszorítások a méretezésekre, és elhelyezésekre, amik widgetek között vannak (ezt lehet, hogy meg kell szorítani, de jó lenne, ha nem kellene).
* A kényszerek az egyik widget valamilyen méret paramétereének függvényében adnak megszorítást egy másik widget méretparaméterére.
* Kérdés: Mi van, ha ellentmondanak a kényszerek?
* Valaki, valahol a kényszerek figyelembevételével végzi el a méretezést és az alignmentet

* Több minden is oda mutat, hogy azok között a widgetek között, akik kölcsön akarnak hatni akár eventek, akár kényszerekkel, kell, hogy legyen közös ősük. Igaz-e a következő: és a megfelelő bejárásoknak először fel kell érniük ide, frissíteni a közös parent állapotát az update során, mielőtt lemennek a másik értesítendő child felé?

* Drag-n-Drop: ez azért érdekes mert itt nem csak két dolog kölcsönhatásáról van szó, hanem min 3 (a külső input [egér], az a widget, amin kezdőtött a művelet, és amin éppen áll az egér). Kérdés, hogy erre ki tudunk-e találni valami általános dolgot, ami általánosítható n ilyen dolog kölcsönhatására? (pl. mi van ha Ctrl+ drag-n-drop-ot akar a felhasználó lekezelni?). Hasonló egyszerűbb esetben a Ctrl+click lekezelése...

* Igaz-e az, hogy a parentek geometriailag szigorúan boundolják a childokat? Ha nem, akkor az egérrel kapcsolatos hit-test-re figyelni kell, illetve a geometriai hierarchia és a logikai hierarchia ekkor eltérő lehet. Lehet, hogy ekkor érdemes egy array-ban tárolni a rect-eket és egyben végig hit-testelni (ez cache hatékonyabb is)?

Első közelítésben nem érdekesek a következő részletek:
- Mikor kell valakit újrarajzolni (mindent mindig újrarajzolunk, majd lehet régiókkal optimalizálni később)
