# FreeRTOS FPGA Seven Segment Display Control

[**Video Demonstrativ pe YouTube**](https://youtu.be/Rv9mmJlA4cA)

Acest proiect implementează un controler pentru două afișaje cu șapte segmente utilizând placa **XILINX XC7S50-CSG324A SPARTAN 7 FPGA**. Aplicația utilizează un temporizator pentru a declanșa actualizări regulate ale afișajelor și gestionează logica acestora folosind un grup de evenimente pentru sincronizarea intrărilor și comunicarea serială.

Pentru mai multe detalii tehnice și explicații, consultă documentația disponibilă în repository: **Documentatie_v2.docx**.

---

## **Descriere Generală**

Proiectul implementează un sistem embedded care:
- Actualizează două afișaje cu șapte segmente la fiecare **500 ms** folosind un temporizator.
- Detectează modificările de stare ale întrerupătoarelor și le reflectă în afișaj.
- Transmite valoarea curentă aflată pe afișaj printr-o interfață de comunicație serială.

Acest sistem este proiectat să ruleze pe un FPGA Spartan 7 folosind un design hardware controlat de FreeRTOS. Dezvoltarea software a fost realizată folosind **Vitis Classic 2023.2**.

---

## **Caracteristici Principale**

1. **Control temporizat**: Actualizare regulată a afișajelor cu șapte segmente.
2. **Detecție a schimbărilor**: Monitorizarea întrerupătoarelor pentru schimbări de stare.
3. **Afișare dinamică**: Valoarea afișată este generată, transmisă și actualizată în timp real.
4. **Sincronizare prin FreeRTOS**: Task-uri separate pentru afișaj, logica întrerupătoarelor și comunicația serială.
5. **Codificare BCD**: Folosirea unui tabel de codificare pentru controlul segmentelor afișajului.

---

## **Arhitectura Sistemului**

### **Blocuri Funcționale**
1. **Temporizator (Timer)**:
   - Declanșează actualizarea afișajelor la fiecare **500 ms**.
2. **Proc1 - Logică întrerupătoare**:
   - Monitorizează schimbările de stare ale întrerupătoarelor.
   - Utilizează un **grup de evenimente** pentru notificare.
3. **Task-uri de trimitere și recepție**:
   - **TxTask**: Generează valori noi și le trimite pentru afișare.
   - **RxTask**: Preia valorile generate și le afișează pe display.

### **Fluxul de Date**
1. Valorile sunt generate de **TxTask**.
2. Sunt transmise către **RxTask** printr-o **coadă**.
3. **RxTask** afișează valoarea și gestionează logica segmentelor pe baza intrărilor din întrerupătoare.

---

## **Hardware Utilizat**

1. **FPGA Spartan 7 (XC7S50-CSG324A)**:
   - Platforma principală de procesare.
2. **Afișaje cu șapte segmente (2 bucăți)**:
   - Tip: Common Cathode.
3. **Module GPIO**:
   - Configurate pentru întrerupătoare și afișaj.
4. **Întrerupătoare**:
   - Pentru schimbarea stării afișajului.

---

## **Software Utilizat**

- **Vitis Classic 2023.2**:
  - Dezvoltarea și depanarea aplicației software.
- **FreeRTOS**:
  - Gestionarea task-urilor și sincronizării evenimentelor.

---

## **Funcționalități Implementate**

1. **Task-uri FreeRTOS**:
   - **TxTask**: Trimiterea valorii generate.
   - **RxTask**: Preluarea valorii și afișarea acesteia.
   - **Proc1**: Monitorizarea și gestionarea întrerupătoarelor.
2. **Temporizator**:
   - Interval de **500 ms** pentru actualizarea afișajelor.
3. **Evenimente și Cozi**:
   - Sincronizare între task-uri folosind grupuri de evenimente și cozi.
4. **Control afișaj**:
   - Codificare BCD pentru afișarea numerelor de la 0 la 99.

---

## **Documentație Suplimentară**

Pentru mai multe informații despre arhitectura proiectului, te rog consultă fișierul **Documentatie_v2.docx** din repository.
