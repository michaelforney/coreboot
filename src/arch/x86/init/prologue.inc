/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2002 Eric Biederman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <cpu/x86/post_code.h>
#include <cpu/x86/stack.h>

.section ".rom.data", "a", @progbits
.section ".rom.text", "ax", @progbits

/* This is the entry code. The code in the .reset section jumps here. */

	post_code(POST_RESET_VECTOR_CORRECT)

