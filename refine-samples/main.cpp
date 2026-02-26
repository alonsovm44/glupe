#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "font.h"
#include "idt.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "kernel.hpp"
#include "3Dgrid.hpp"
#include "commands.hpp"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

// Variable global para que las excepciones puedan dibujar en pantalla
static struct limine_framebuffer *global_fb = nullptr;

// Global pointer to command history for access by commands
Vector<String>* command_history_ptr = nullptr;

// --- Hardware I/O ---
void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

void outw(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline void enable_interrupts() {
    asm volatile("sti");
}

static inline void disable_interrupts() {
    asm volatile("cli");
}

void halt_cpu() {
    asm volatile("hlt");
}

// --- OPTIMIZACIÓN DE CPU: SLEEP ---
void sleep_approx(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        asm volatile("pause");
    }
}

// --- PS/2 Mouse Driver ---
// (Restaurado completo)
static volatile uint8_t mouse_buffer[128]; // Forward definition for buffer
uint8_t mouse_packet_size = 3;

void mouse_wait(uint8_t type) {
    int timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; asm volatile("pause"); }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; asm volatile("pause"); }
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
    mouse_wait(0); // Wait for ACK
    inb(0x60);     // Consume ACK
}

void mouse_init() {
    uint8_t status;

    // 1. Habilitar la entrada auxiliar (mouse)
    mouse_wait(1);
    outb(0x64, 0xA8);

    // 2. Leer el "Comand Byte" actual
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = inb(0x60) | 2; // Habilitar bit de interrupción del mouse (IRQ12)
    status &= ~(1 << 5);    // Asegurar que el mouse no esté deshabilitado (bit 5)

    // 3. Escribir el nuevo "Comand Byte"
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // 4. Enable Scroll Wheel (IntelliMouse)
    mouse_write(0xF6); // Defaults
    mouse_write(0xF3); mouse_write(200);
    mouse_write(0xF3); mouse_write(100);
    mouse_write(0xF3); mouse_write(80);
    
    mouse_write(0xF2); // Get ID
    mouse_wait(0);
    uint8_t id = inb(0x60);
    if (id == 3) mouse_packet_size = 4;
    else mouse_packet_size = 3;

    // 5. Enable data reporting
    mouse_write(0xF4); 
}

// --- PC Speaker Driver ---
void beep(uint32_t freq, uint32_t duration_ms) {
    if (freq == 0) {
        sleep_ms(duration_ms);
        return;
    }
    uint32_t divisor = 1193180 / freq;
    
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }

    sleep_ms(duration_ms);

    tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

// --- Graphics Engine ---
static uint32_t* g_backbuffer = nullptr;

void fast_memcpy(void* dst, const void* src, size_t size) {
    uint64_t* d = (uint64_t*)dst;
    const uint64_t* s = (const uint64_t*)src;
    size_t n = size / 8;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    
    uint8_t* d8 = (uint8_t*)dst;
    const uint8_t* s8 = (const uint8_t*)src;
    for (size_t i = n * 8; i < size; i++) d8[i] = s8[i];
}

void init_graphics(struct limine_framebuffer* fb) {
    global_fb = fb; // Guardamos referencia global para Panic Handler
    if (g_backbuffer) return;
    g_backbuffer = (uint32_t*)malloc(fb->width * fb->height * 4);
}

uint32_t* get_backbuffer() { return g_backbuffer; }

void swap_buffers(struct limine_framebuffer* fb) {
    if (g_backbuffer) fast_memcpy(fb->address, g_backbuffer, fb->width * fb->height * 4);
}

void draw_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color) {
    if (x >= 0 && x < (int)fb->width && y >= 0 && y < (int)fb->height) {
        uint32_t* buffer = g_backbuffer ? g_backbuffer : (uint32_t*)fb->address;
        buffer[y * fb->width + x] = color;
    }
}

void draw_char(struct limine_framebuffer *fb, int x, int y, char c, uint32_t color) {
    uint8_t uc = (uint8_t)c;
    if (uc < FONT_FIRST_CHAR || uc > FONT_LAST_CHAR) return;
    uint8_t index = (uc - FONT_FIRST_CHAR);
    if (index >= FONT_GLYPH_COUNT) return;

    const uint8_t* glyph = font8x8_basic[index];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                draw_pixel(fb, x + col, y + row, color);
            }
        }
    }
}

void draw_string(struct limine_framebuffer *fb, int x, int y, const char* str, uint32_t color) {
    int cursor_x = x;
    while (*str) {
        draw_char(fb, cursor_x, y, *str, color);
        cursor_x += 8;
        str++;
    }
}

// NUEVA FUNCION: Para convertir uint64 a hex string de forma segura
void u64_to_hex(uint64_t val, char* dest) {
    const char* hex_chars = "0123456789ABCDEF";
    dest[0] = '0'; dest[1] = 'x';
    for (int i = 0; i < 16; i++) {
        dest[17 - i] = hex_chars[(val >> (i * 4)) & 0xF];
    }
    dest[18] = '\0';
}

void itoa(int value, char* buffer, int base) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    int tmp_value;

    if (value == 0) { *ptr++ = '0'; *ptr = '\0'; return; }
    if (value < 0 && base == 10) { *ptr++ = '-'; ptr1++; value = -value; }

    while (value > 0) {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789ABCDEF"[tmp_value - (value * base)];
    }
    *ptr-- = '\0';
    while (ptr1 < ptr) { tmp_char = *ptr; *ptr-- = *ptr1; *ptr1++ = tmp_char; }
}

extern "C" void shutdown() {
    outw(0x604, 0x2000); // QEMU
    outw(0x4004, 0x3400); // VirtualBox
    outw(0xB004, 0x2000); // Bochs
}

extern "C" void reboot() {
    outb(0x64, 0xFE);
    halt_cpu();
}

char scancode_to_char(uint8_t sc, bool shift, bool caps) {
    static const char map_lower[128] = {
        0,  0, '1','2','3','4','5','6','7','8','9','0','-','=', 0,
        0, 'q','w','e','r','t','y','u','i','o','p','[',']', '\n',
        0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
        0, '\\','z','x','c','v','b','n','m',',','.','/', 0, '*',
        0, ' '
    };
    static const char map_upper[128] = {
        0,  0, '!','@','#','$','%','^','&','*','(',')','_','+', 0,
        0, 'Q','W','E','R','T','Y','U','I','O','P','{','}', '\n',
        0, 'A','S','D','F','G','H','J','K','L',':','\"','~',
        0, '|','Z','X','C','V','B','N','M','<','>','?', 0, '*',
        0, ' '
    };

    if (sc >= 128) return 0;

    char c = map_lower[sc];
    if (c >= 'a' && c <= 'z') {
        return (shift != caps) ? map_upper[sc] : c;
    }
    return shift ? map_upper[sc] : c;
}

void enable_sse() {
    uint64_t cr0, cr4;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); 
    cr0 |= (1 << 1);  
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);  
    cr4 |= (1 << 10); 
    asm volatile ("mov %0, %%cr4" :: "r"(cr4));
}

// --- Interrupt Handling ---
__attribute__((aligned(0x10))) 
static idt_entry idt[256];
static idtr idtr_desc;

// Input Buffers
#define RING_BUFFER_SIZE 128
static volatile uint8_t key_buffer[RING_BUFFER_SIZE];
static volatile uint16_t key_head = 0;
static volatile uint16_t key_tail = 0;

static volatile uint16_t mouse_head = 0;
static volatile uint16_t mouse_tail = 0;

static volatile uint64_t ticks = 0;
static uint32_t terminal_color = 0xFFFFFFFF;
static uint32_t terminal_bg_color = 0xFF000000;

bool has_key() { return key_head != key_tail; }
uint8_t get_key() {
    if (key_head == key_tail) return 0;
    uint8_t c = key_buffer[key_tail];
    key_tail = (key_tail + 1) % RING_BUFFER_SIZE;
    return c;
}

bool has_mouse() { return mouse_head != mouse_tail; }
uint8_t get_mouse_byte() {
    if (mouse_head == mouse_tail) return 0;
    uint8_t b = mouse_buffer[mouse_tail];
    mouse_tail = (mouse_tail + 1) % RING_BUFFER_SIZE;
    return b;
}

uint64_t get_ticks() { return ticks; }

void sleep_ms(uint64_t ms) {
    uint64_t target = ticks + ms; // Assuming 1000Hz timer
    while (ticks < target) {
        halt_cpu();
    }
}

void set_terminal_color(uint32_t color) { terminal_color = color; }
void set_terminal_bg_color(uint32_t color) { terminal_bg_color = color; }
uint32_t get_terminal_bg_color() { return terminal_bg_color; }

// ISRs
__attribute__((interrupt)) void timer_handler(struct interrupt_frame* frame) {
    ticks++;
    outb(0x20, 0x20); // EOI
}

__attribute__((interrupt)) void keyboard_handler(struct interrupt_frame* frame) {
    uint8_t scancode = inb(0x60);
    uint16_t next_head = (key_head + 1) % RING_BUFFER_SIZE;
    if (next_head != key_tail) {
        key_buffer[key_head] = scancode;
        key_head = next_head;
    }
    outb(0x20, 0x20); // EOI
}

__attribute__((interrupt)) void mouse_handler(struct interrupt_frame* frame) {
    uint8_t byte = inb(0x60);
    uint16_t next_head = (mouse_head + 1) % RING_BUFFER_SIZE;
    if (next_head != mouse_tail) {
        mouse_buffer[mouse_head] = byte;
        mouse_head = next_head;
    }
    outb(0xA0, 0x20); // EOI Slave
    outb(0x20, 0x20); // EOI Master
}

// CORRECCIÓN 1: Manejador de Page Fault para evitar reinicios infinitos
__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame* frame) {
    uint64_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
    
    // Si tenemos pantalla gráfica, dibujamos el error
    if (global_fb) {
        draw_string(global_fb, 10, 10, "!!! KERNEL PANIC: PAGE FAULT !!!", 0xFFFF0000);
        
        char buf[32];
        u64_to_hex(fault_addr, buf);
        draw_string(global_fb, 10, 30, "Address: ", 0xFFFFFFFF);
        draw_string(global_fb, 82, 30, buf, 0xFFFFFF00);
        
        draw_string(global_fb, 10, 50, "System Halted.", 0xFFFFFFFF);
        
        if (g_backbuffer) swap_buffers(global_fb);
    }
    
    // Bucle infinito para detener el OS
    while(1) { asm volatile("hlt"); }
}

void remap_pic() {
    uint8_t a1 = inb(0x21);
    uint8_t a2 = inb(0xA1);

    outb(0x20, 0x11); outb(0xA0, 0x11); // Start init
    outb(0x21, 0x20); outb(0xA1, 0x28); // Vector offsets (32, 40)
    outb(0x21, 0x04); outb(0xA1, 0x02); // Cascade
    outb(0x21, 0x01); outb(0xA1, 0x01); // 8086 mode

    // Unmask IRQ0 (Timer), IRQ1 (Keyboard), IRQ2 (Cascade), IRQ12 (Mouse)
    outb(0x21, 0xF8); 
    outb(0xA1, 0xEF); 
}

void set_idt_gate(int n, uint64_t handler) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = cs;
    idt[n].ist = 0;
    idt[n].types_attr = 0x8E; // Interrupt Gate, Present, DPL0
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

void init_interrupts() {
    remap_pic();

    set_idt_gate(32, (uint64_t)timer_handler);
    set_idt_gate(33, (uint64_t)keyboard_handler);
    set_idt_gate(44, (uint64_t)mouse_handler);
    
    // REGISTRO DEL PAGE FAULT (Excepción 14)
    set_idt_gate(14, (uint64_t)page_fault_handler);

    idtr_desc.limit = sizeof(idt) - 1;
    idtr_desc.base = (uint64_t)&idt;
    asm volatile("lidt %0" : : "m"(idtr_desc));

    // Configure PIT to ~1000Hz
    uint16_t divisor = 1193;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    enable_interrupts();
}

// --- Memory Management --- (Tal cual estaba)
// (Implementación de malloc/free omitida para brevedad en el chat pero asumida presente por el linker si estaba en memory.cpp)
// Si estaba inline en main.cpp, deberías mantenerla.

// --- Terminal Logic ---
void term_print(Vector<String>& buffer, const char* text, int max_chars) {
    String line;
    int line_len = 0;
    
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if (c == '\n') {
            buffer.push_back(line);
            line.clear();
            line_len = 0;
        } else {
            if (line_len + 1 > max_chars) {
                // Word wrapping básico
                int split_idx = -1;
                const char* l_str = line.c_str();
                for (int k = line_len - 1; k >= 0; k--) {
                    if (l_str[k] == ' ') {
                        split_idx = k;
                        break;
                    }
                }
                
                if (split_idx != -1) {
                    String next_line;
                    for (int k = split_idx + 1; k < line_len; k++) {
                        next_line.push_back(l_str[k]);
                    }
                    next_line.push_back(c);
                    
                    String current_line_final;
                    for (int k = 0; k < split_idx; k++) current_line_final.push_back(l_str[k]);
                    buffer.push_back(current_line_final);
                    
                    line = next_line;
                    line_len = line.length();
                } else {
                    buffer.push_back(line);
                    line.clear();
                    line.push_back(c);
                    line_len = 1;
                }
            } else {
                line.push_back(c);
                line_len++;
            }
        }
    }
    if (line_len > 0) buffer.push_back(line);
}

void start_terminal_mode(struct limine_framebuffer *fb) {
    String cmd_buf;
    bool dirty = true;
    
    Vector<String> history;
    command_history_ptr = &history;
    Vector<String> text_buffer;
    int history_index = -1;
    bool is_extended = false;
    uint64_t last_cursor_blink = 0;
    bool cursor_visible = true;
    bool cursor_needs_update = false;
    int cursor_x = 0;
    int cursor_y = 0;
    bool full_refresh = true;
    bool shift = false;
    bool caps = false;

    int line_height = 16;
    int max_lines = fb->height / line_height;
    int scroll_offset = 0;
    int max_chars = (fb->width - 20) / 8;

    term_print(text_buffer, "Mark Ji OS u7.3", max_chars);
    term_print(text_buffer, "(C) 2026 Alonso V.M", max_chars);
    term_print(text_buffer, "Type 'pov' for 3D demo", max_chars);

    // Initial clear
    uint32_t* buffer = g_backbuffer ? g_backbuffer : (uint32_t*)fb->address;
    for (uint64_t i = 0; i < fb->width * fb->height; i++) buffer[i] = terminal_bg_color;
    if (g_backbuffer) swap_buffers(fb);

    while (1) {
        halt_cpu(); 
        
        if (get_ticks() - last_cursor_blink >= 500) {
            cursor_visible = !cursor_visible;
            last_cursor_blink = get_ticks();
            cursor_needs_update = true;
        }

        while (has_key()) {
            cursor_visible = true;
            last_cursor_blink = get_ticks();
            uint8_t scancode = get_key();
            
            if (scancode == 0xE0) { is_extended = true; continue; }
            if (scancode == 0x2A || scancode == 0x36) { shift = true; continue; }
            if (scancode == 0xAA || scancode == 0xB6) { shift = false; continue; }
            if (scancode == 0x3A) { caps = !caps; continue; }
            if (scancode & 0x80) { is_extended = false; continue; }

            if (is_extended) {
                if (scancode == 0x48) { // Up
                    if (history.size() > 0) {
                        if (history_index == -1) history_index = (int)history.size() - 1;
                        else if (history_index > 0) history_index--;
                        cmd_buf = history[history_index];
                        dirty = true; scroll_offset = 0;
                    }
                } else if (scancode == 0x50) { // Down
                    if (history_index != -1) {
                        if (history_index < (int)history.size() - 1) {
                            history_index++;
                            cmd_buf = history[history_index];
                        } else {
                            history_index = -1;
                            cmd_buf.clear();
                        }
                        dirty = true; scroll_offset = 0;
                    }
                } else if (scancode == 0x49) { // Page Up
                    scroll_offset += max_lines;
                    dirty = true;
                } else if (scancode == 0x51) { // Page Down
                    scroll_offset -= max_lines;
                    dirty = true;
                }
                is_extended = false;
                continue;
            }

            if (scancode == 0x1C) { // Enter
                full_refresh = true;
                scroll_offset = 0;
                if (cmd_buf.length() > 0) history.push_back(cmd_buf);
                history_index = -1;

                String input_line = "> ";
                input_line.append(cmd_buf.c_str());
                term_print(text_buffer, input_line.c_str(), max_chars);

                const char* raw_cmd = cmd_buf.c_str();
                char cmd_name[64];
                int name_len = 0;
                const char* args = "";

                while (raw_cmd[name_len] && raw_cmd[name_len] != ' ' && name_len < 63) {
                    cmd_name[name_len] = raw_cmd[name_len];
                    name_len++;
                }
                cmd_name[name_len] = '\0';

                if (raw_cmd[name_len] == ' ') args = raw_cmd + name_len + 1;

                String output;
                bool found = false;
                for (int i = 0; command_registry[i].name; i++) {
                    const char* reg_name = command_registry[i].name;
                    bool match = true;
                    int j = 0;
                    while (cmd_name[j] && reg_name[j]) {
                        if (cmd_name[j] != reg_name[j]) { match = false; break; }
                        j++;
                    }
                    if (match && !cmd_name[j] && !reg_name[j]) {
                        command_registry[i].handler(fb, args, output);
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    bool is_clear = true;
                    const char* c = "clear";
                    int k = 0;
                    while(cmd_name[k] && c[k] && cmd_name[k] == c[k]) k++;
                    if (cmd_name[k] != 0 || c[k] != 0) is_clear = false;

                    if (is_clear) text_buffer.clear();
                    else if (output.length() > 0) term_print(text_buffer, output.c_str(), max_chars);
                } else if (cmd_buf.length() > 0) {
                    term_print(text_buffer, "Unknown command. Type 'help'.", max_chars);
                }
                cmd_buf.clear(); dirty = true;
            } else if (scancode == 0x0E) { 
                if (cmd_buf.length() > 0) { cmd_buf.pop_back(); dirty = true; scroll_offset = 0; }
            } else {
                char c = scancode_to_char(scancode, shift, caps);
                if (c) { cmd_buf.push_back(c); dirty = true; scroll_offset = 0; }
            }
        }

        if (dirty) {
            String full_input = "> ";
            full_input.append(cmd_buf.c_str());
            Vector<String> input_lines;
            term_print(input_lines, full_input.c_str(), max_chars);
            if (input_lines.size() == 0) input_lines.push_back(full_input);

            int total_buffer_lines = (int)text_buffer.size();
            int start_idx = 0;
            int total_visual_lines = total_buffer_lines + (int)input_lines.size(); 
            
            if (total_visual_lines > max_lines) {
                // Ajuste scroll con offset
                int max_scroll_possible = total_visual_lines - max_lines;
                if (scroll_offset > max_scroll_possible) scroll_offset = max_scroll_possible;
                start_idx = max_scroll_possible - scroll_offset;
            } else {
                scroll_offset = 0;
            }
            if (start_idx < 0) start_idx = 0;
            
            int y = 0;

            if (full_refresh) {
                size_t pixel_count = fb->width * fb->height;
                uint32_t* pixels = g_backbuffer ? g_backbuffer : (uint32_t*)fb->address;
                for (size_t i = 0; i < pixel_count; i++) pixels[i] = terminal_bg_color;

                int start_buffer = start_idx;
                if (start_buffer >= (int)text_buffer.size()) start_buffer = (int)text_buffer.size();

                for (int i = start_buffer; i < (int)text_buffer.size(); i++) {
                    if (y + line_height > (int)fb->height) break;
                    draw_string(fb, 10, y, text_buffer[i].c_str(), terminal_color);
                    y += line_height;
                }
            } else {
                // --- CORRECCIÓN 2: Lógica Dirty Robusta ---
                int lines_from_buffer = 0;
                if (start_idx < (int)text_buffer.size()) {
                    lines_from_buffer = (int)text_buffer.size() - start_idx;
                }
                // Si el buffer ocupa más que la pantalla, acotamos
                int max_visible_lines = fb->height / line_height;
                if (lines_from_buffer > max_visible_lines) lines_from_buffer = max_visible_lines;

                y = lines_from_buffer * line_height;
                
                // Limpiar desde donde empieza el input hasta el final de la pantalla
                // Esto elimina "basura" visual de comandos anteriores largos
                uint32_t* pixels = g_backbuffer ? g_backbuffer : (uint32_t*)fb->address;
                int clear_start_y = y;
                if (clear_start_y < (int)fb->height) {
                    int clear_rows = fb->height - clear_start_y;
                    size_t offset = clear_start_y * fb->width;
                    size_t count = clear_rows * fb->width;
                    for (size_t i = 0; i < count; i++) {
                        pixels[offset + i] = terminal_bg_color;
                    }
                }
            }

            // Dibujar Input
            int input_start_idx = 0;
            // Si el scroll nos comió parte del input
            if (start_idx > (int)text_buffer.size()) {
                 input_start_idx = start_idx - (int)text_buffer.size();
            }

            for (int i = input_start_idx; i < (int)input_lines.size(); i++) {
                if (y + line_height > (int)fb->height) break;
                draw_string(fb, 10, y, input_lines[i].c_str(), terminal_color);
                
                // --- CORRECCIÓN 3: Guardar posición correcta del cursor ---
                // Calculamos dónde debe ir el cursor mientras dibujamos la línea
                if (i == (int)input_lines.size() - 1) {
                    int last_len = input_lines[i].length();
                    // Caso borde: si la línea llena exactamente el ancho, el cursor baja
                    if (last_len >= max_chars) {
                        cursor_x = 10;
                        cursor_y = y + line_height; // Siguiente renglón
                    } else {
                        cursor_x = 10 + last_len * 8;
                        cursor_y = y; // Mismo renglón
                    }
                }
                y += line_height;
            }
            
            // Si no hay input lines (raro), cursor al inicio
            if (input_lines.size() == 0) { cursor_x = 10; cursor_y = y; }

            if (cursor_visible) {
                draw_string(fb, cursor_x, cursor_y, "_", terminal_color);
            }

            dirty = false;
            if (g_backbuffer) swap_buffers(fb);
            full_refresh = false;
            cursor_needs_update = false;
        } else if (cursor_needs_update) {
            if (cursor_visible) {
                draw_string(fb, cursor_x, cursor_y, "_", terminal_color);
            } else {
                for (int dy = 0; dy < 8; dy++) {
                    for (int dx = 0; dx < 8; dx++) {
                        draw_pixel(fb, cursor_x + dx, cursor_y + dy, terminal_bg_color); 
                    }
                }
            }
            if (g_backbuffer) swap_buffers(fb);
            cursor_needs_update = false;
        }
    }
}

// Necesitamos definir xtoi aquí para que esté disponible si no lo está en string.hpp
uint64_t xtoi(const char* str) {
    uint64_t res = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    while (*str) {
        uint8_t v = 0;
        if (*str >= '0' && *str <= '9') v = *str - '0';
        else if (*str >= 'a' && *str <= 'f') v = *str - 'a' + 10;
        else if (*str >= 'A' && *str <= 'F') v = *str - 'A' + 10;
        else break;
        res = (res << 4) | v;
        str++;
    }
    return res;
}

extern "C" void* get_module_file(const char* name, size_t* size) {
    if (!module_request.response || !module_request.response->modules) return nullptr;
    
    for (uint64_t i = 0; i < module_request.response->module_count; i++) {
        struct limine_file* f = module_request.response->modules[i];
        char* path = f->path;
        if (!path) continue;
        
        // Check if path ends with name (e.g. "boot:///boot/welcome.txt" ends with "welcome.txt")
        int path_len = 0; while(path[path_len]) path_len++;
        int name_len = 0; while(name[name_len]) name_len++;
        
        if (path_len >= name_len) {
            bool match = true;
            for(int j=0; j<name_len; j++) {
                if (path[path_len - name_len + j] != name[j]) { match = false; break; }
            }
            if (match) {
                if (size) *size = f->size;
                return f->address;
            }
        }
    }
    return nullptr;
}

extern "C" void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        for (;;);
    }
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    enable_sse();
    mouse_init(); 
    init_graphics(fb);
    init_interrupts(); 
    start_terminal_mode(fb);
}