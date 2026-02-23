# GLUPE_BLOCK_START: legacy_cobol_transpile
# This Python script is a native re‑implementation of the COBOL logic
# found in experiments/cobol/legacy.cbl.glp.
#
# Requirements from user:
# - Single Python file
# - No external calls to other languages
# - Preserve GLUPE markers
# - Implement logic between markers
# - Self‑contained and runnable

import os
from decimal import Decimal, InvalidOperation

INPUT_FILENAME = "INPUT.DAT"

def read_input_file(path):
    """
    Generator that yields parsed records from INPUT.DAT.
    COBOL layout:
        05 CUSTOMER-ID      PIC X(10)
        05 CUSTOMER-NAME    PIC X(20)
        05 BALANCE          PIC 9(5)V99  (implied decimal)
    """
    if not os.path.exists(path):
        return

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if len(line) < 10 + 20 + 7:
                # Skip malformed lines
                continue

            cust_id = line[0:10]
            cust_name = line[10:30]
            balance_raw = line[30:37]  # 5 digits + 2 decimals

            try:
                # Convert "1234501" -> "12345.01"
                balance = Decimal(balance_raw[:-2] + "." + balance_raw[-2:])
            except (InvalidOperation, ValueError):
                balance = Decimal("0.00")

            yield {
                "CUSTOMER_ID": cust_id.strip(),
                "CUSTOMER_NAME": cust_name.strip(),
                "BALANCE": balance
            }


def main():
    # COBOL working-storage equivalents
    ws_eof = False
    ws_total = Decimal("0.00")
    ws_count = 0

    # OPEN INPUT INPUT-FILE
    records = read_input_file(INPUT_FILENAME)
    records = iter(records)

    # READ-FILE (first read)
    try:
        current_record = next(records)
    except StopIteration:
        ws_eof = True
        current_record = None

    # PROCESS-FILE UNTIL EOF
    while not ws_eof:
        # ADD BALANCE TO WS-TOTAL
        ws_total += current_record["BALANCE"]

        # ADD 1 TO WS-COUNT
        ws_count += 1

        # PERFORM READ-FILE
        try:
            current_record = next(records)
        except StopIteration:
            ws_eof = True
            current_record = None

    # PRINT-TOTAL
    print("TOTAL CUSTOMERS:", ws_count)
    print("TOTAL BALANCE:", ws_total)

    # CLOSE INPUT-FILE (implicit by generator exhaustion)


if __name__ == "__main__":
    main()

# GLUPE_BLOCK_END: legacy_cobol_transpile
