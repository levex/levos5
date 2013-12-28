#include <stdint.h>
#include <hal.h>
#include <keyboard.h>
#include <mm.h>
#include <input.h>

uint8_t *keybuf = 0;
uint16_t key_loc = 0;
uint8_t lastkey = 0;
uint8_t kbd_enabled = 0;
uint8_t keys_avail = 0;

enum KEYCODE {
        NULL_KEY = 0,
        Q_PRESSED = 0x10,
        Q_RELEASED = 0x90,
        W_PRESSED = 0x11,
        W_RELEASED = 0x91,
        E_PRESSED = 0x12,
        E_RELEASED = 0x92,
        R_PRESSED = 0x13,
        R_RELEASED = 0x93,
        T_PRESSED = 0x14,
        T_RELEASED = 0x94,
        Z_PRESSED = 0x15,
        Z_RELEASED = 0x95,
        U_PRESSED = 0x16,
        U_RELEASED = 0x96,
        I_PRESSED = 0x17,
        I_RELEASED = 0x97,
        O_PRESSED = 0x18,
        O_RELEASED = 0x98,
        P_PRESSED = 0x19,
        P_RELEASED = 0x99,
        A_PRESSED = 0x1E,
        A_RELEASED = 0x9E,
        S_PRESSED = 0x1F,
        S_RELEASED = 0x9F,
        D_PRESSED = 0x20,
        D_RELEASED = 0xA0,
        F_PRESSED = 0x21,
        F_RELEASED = 0xA1,
        G_PRESSED = 0x22,
        G_RELEASED = 0xA2,
        H_PRESSED = 0x23,
        H_RELEASED = 0xA3,
        J_PRESSED = 0x24,
        J_RELEASED = 0xA4,
        K_PRESSED = 0x25,
        K_RELEASED = 0xA5,
        L_PRESSED = 0x26,
        L_RELEASED = 0xA6,
        Y_PRESSED = 0x2C,
        Y_RELEASED = 0xAC,
        X_PRESSED = 0x2D,
        X_RELEASED = 0xAD,
        C_PRESSED = 0x2E,
        C_RELEASED = 0xAE,
        V_PRESSED = 0x2F,
        V_RELEASED = 0xAF,
        B_PRESSED = 0x30,
        B_RELEASED = 0xB0,
        N_PRESSED = 0x31,
        N_RELEASED = 0xB1,
        M_PRESSED = 0x32,
        M_RELEASED = 0xB2,

        ZERO_PRESSED = 0x29,
        ONE_PRESSED = 0x2,
        NINE_PRESSED = 0xA,

        POINT_PRESSED = 0x34,
        POINT_RELEASED = 0xB4,

        SLASH_RELEASED = 0xB5,

        BACKSPACE_PRESSED = 0x0E,
        BACKSPACE_RELEASED = 0x8E,
        SPACE_PRESSED = 0x39,
        SPACE_RELEASED = 0xB9,
        ENTER_PRESSED = 0x1C,
        ENTER_RELEASED = 0x9C,

};

static char c = 0;
char keyboard_get_key()
{
        c = 0;
        if(key_loc == 0) goto out;
        c = *keybuf;
        key_loc --;
        for(int i = 0; i < 256; i++)
        {
                keybuf[i] = keybuf[i+1];
        }
        keys_avail --;
out:
        return c;
}
static char* _qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";
uint8_t keyboard_to_ascii(uint8_t key)
{
        if(key == 0x1C) return '\n';
        if(key == 0x39) return ' ';
        if(key == 0xE) return '\r';
        if(key == POINT_RELEASED) return '.';
	if(key == BACKSPACE_PRESSED) return ' ';
        if(key == SLASH_RELEASED) return '/';
        if(key == ZERO_PRESSED) return '0';
        if(key >= ONE_PRESSED && key <= NINE_PRESSED)
                return _num[key - ONE_PRESSED];
        if(key >= 0x10 && key <= 0x1C)
        {
                return _qwertzuiop[key - 0x10];
        } else if(key >= 0x1E && key <= 0x26)
        {
                return _asdfghjkl[key - 0x1E];
        } else if(key >= 0x2C && key <= 0x32)
        {
                return _yxcvbnm[key - 0x2C];
        }
        return 0;
}

void keyboard_read_key()
{
        lastkey = 0;
        if(inportb(0x64) & 1)
                lastkey = inportb(0x60);
}

int kbd_read(struct input *m, uint8_t *buf, uint32_t len)
{
	m = m;
	int to_read = (len > keys_avail ? keys_avail : len);
	for(int i = 0; i < to_read; i++)
	{
		buf[i] = (uint8_t)keyboard_get_key();
	}
	return to_read;
}

struct input keyboard_input = {
	.read = kbd_read,
	.id = 0,
};

struct input *keyboard_create_new(struct tty *mtty)
{
	struct input *cp = malloc(sizeof(struct input));
	memcpy((uint8_t *)cp, (uint8_t *)&keyboard_input, sizeof(struct input));
	cp->mtty = mtty;
	return cp;
}

void kbd_irq()
{
		IRQ_START;
		char c = keyboard_to_ascii(inportb(0x60));
		if(c) {
			keybuf[key_loc++] = c;
			keys_avail ++;
		}
        send_eoi(1);
        IRQ_END;
}

int keyboard_init()
{
	keybuf = malloc(256);
	register_interrupt(33, kbd_irq);
	input_register(&keyboard_input);
	kbd_enabled = 1;
	return 0;
}
