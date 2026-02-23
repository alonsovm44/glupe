use std::env;
use std::fs;
use std::io;

static mut BUFFER: Vec<u8> = Vec::new();

fn load_file(filename: &str) {
    let data = fs::read(filename);
    match data {
        Ok(bytes) => unsafe {
            BUFFER = bytes;
        },
        Err(_) => {
            eprintln!("Error: Cannot open file.");
        }
    }
}

fn count_lines() -> usize {
    unsafe { BUFFER.iter().filter(|&&c| c == b'\n').count() }
}

fn count_words() -> usize {
    let mut words = 0usize;
    let mut in_word = false;

    unsafe {
        for &c in BUFFER.iter() {
            if c.is_ascii_whitespace() {
                if in_word {
                    words += 1;
                    in_word = false;
                }
            } else {
                in_word = true;
            }
        }
    }

    if in_word {
        words += 1;
    }

    words
}

fn find_pattern(pattern: &str) -> usize {
    let pat = pattern.as_bytes();
    let pat_len = pat.len();
    let mut count = 0usize;

    unsafe {
        if BUFFER.len() < pat_len {
            return 0;
        }

        for i in 0..=BUFFER.len() - pat_len {
            if &BUFFER[i..i + pat_len] == pat {
                count += 1;
            }
        }
    }

    count
}

fn generate_report() {
    let lines = count_lines();
    let words = count_words();
    let patterns = find_pattern("pattern");

    println!("----- REPORT START -----");
    println!("Lines: {}", lines);
    println!("Words: {}", words);
    println!("Pattern occurrences: {}", patterns);
    println!("------ REPORT END ------");
}

fn process_data() {
    unsafe {
        if BUFFER.is_empty() {
            return;
        }
    }

        // Original logic: compute lines, words, patterns, then call generate_report()
    let _lines = count_lines();
    let _words = count_words();
    let _patterns = find_pattern("pattern");
    generate_report();
    }

fn cleanup() {
    unsafe {
        BUFFER.clear();
    }
}

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: ./program <filename>");
        return Ok(());
    }

    load_file(&args[1]);
    process_data();
    cleanup();

    Ok(())
}
