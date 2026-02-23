       IDENTIFICATION DIVISION.
       PROGRAM-ID. LEGACY-SPAGHETTI.
       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
           SELECT INPUT-FILE ASSIGN TO "INPUT.DAT".
       DATA DIVISION.
       FILE SECTION.
       FD INPUT-FILE.
       01 INPUT-RECORD.
           05 CUSTOMER-ID      PIC X(10).
           05 CUSTOMER-NAME    PIC X(20).
           05 BALANCE          PIC 9(5)V99.
       WORKING-STORAGE SECTION.
       01 WS-EOF               PIC X VALUE 'N'.
           88 EOF              VALUE 'Y'.
       01 WS-TOTAL             PIC 9(9)V99 VALUE 0.
       01 WS-COUNT             PIC 9(5) VALUE 0.

       PROCEDURE DIVISION.
       MAIN-PARA.
           OPEN INPUT INPUT-FILE.
           PERFORM READ-FILE.
           PERFORM PROCESS-FILE UNTIL EOF.
           PERFORM PRINT-TOTAL.
           CLOSE INPUT-FILE.
           STOP RUN.

       READ-FILE.
           READ INPUT-FILE
               AT END MOVE 'Y' TO WS-EOF
           END-READ.

       PROCESS-FILE.
           ADD BALANCE TO WS-TOTAL.
           ADD 1 TO WS-COUNT.
           PERFORM READ-FILE.

       PRINT-TOTAL.
           DISPLAY "TOTAL CUSTOMERS: " WS-COUNT.
           DISPLAY "TOTAL BALANCE: " WS-TOTAL.