Heliostat
----------
Celem projektu było stworzenie działającego heliostatu, który ustawia lustro pod odpowiednim kątem na podstawie obliczanej ścieżki Słońca i odbija promienie słoneczne w jeden, wybrany punkt.

Po stronie programistycznej został napisany kod, który liczy pozycję Słońca na podstawie aktualnej pozycji w koordynatach oraz czasu, który jest sczytywany z zegara czasu rzeczywistego zaimplementowanego w projekcie. 
Najpierw liczony jest __czas lokalny__, który zawarty jest w minutach dla ułatwienia obliczeń. Do zmiennych wpisane są koordynaty w postaci długości i szerokości geograficznej. 
Liczony jest __czas lokalny z uwzględnieniem strefy czasowej__. Następnie liczony jest aktualny dzień od początku roku oraz __równanie czasu__ na podstawie tego dnia, 
które koryguje nachylenie osi Ziemi i to, że orbita Ziemi jest eliptyczna. Liczony jest __współczynnik korekcji czasu__, który uwzględnia zmiany lokalnego czasu słonecznego w danej 
strefie czasowej wynikające ze zmian długości geograficznej w obrębie strefy czasowej. Następnie liczony jest __lokalny czas słoneczny__, który wykorzystuje współczynnik korekcji czasu i
czas lokalny. Na podstawie tego liczony jest __kąt godzinny__, który przekształca lokalny czas słoneczny na liczbę stopni, o jaką słońce porusza się po niebie. Z definicji kąt godzinny wynosi 0°
w południe słoneczne. Ponieważ Ziemia obraca się o 15° na godzinę, każda godzina poza południem słonecznym odpowiada ruchowi kątowemu słońca na niebie o 15°. Rano kąt godzinny jest ujemny,
po południu kąt godzinny jest dodatni. Dalej liczymy __deklinację__, która jest kątem pomiędzy równikiem a Słońcem. Wynika ona z przekrzywienia Ziemi w osi jej obrotu. Na końcu
liczony jest __kąt wysokości Słońca__ oraz __azymut__. Są to kąty które można porównać do współrzędnych X i Y Słońca, na ich podstawie znane jest dokładne położenie Słońca z danej
lokalizacji i w danym czasie. Wszystkie obliczenia wyświetlane są w konsoli Arduino IDE w celu weryfikacji poprawności obliczeń.

Wykorzystane komponenty: Arduino Uno, 2x silnik krokowy, 2x sterownik A4988, DS1307 RTC, źródło zasilania 24V.

