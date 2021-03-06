/*************************************************************
 * Copyright (c) 2006 Shotaro Tsuji
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is     * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *************************************************************/

/* Please send bug reports to
    Shotaro Tsuji
    4-1010,
    Sakasedai 1-chome,
    Takaraduka-si,
    Hyogo-ken,
    665-0024
    Japan
    negi4d41@yahoo.co.jp
*/

#ifndef _ERRNO_H_
#define _ERRNO_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern int *__error(void);
#define errno (*__error())

#ifndef EUNKNOWN
#define EUNKNOWN 1 /* Unkonwn error */
#endif

#ifndef EPERM
#define EPERM 2 /* Operation not permitted */
#endif

#ifndef EBADF
#define EBADF 3 /* Bad file descriptor */
#endif

#ifndef ENOMEM
#define ENOMEM 4 /* Cannot allocate memory */
#endif

#ifndef EINVAL
#define EINVAL 5 /* Invalid argument */
#endif

#ifndef EDOM
#define EDOM 6  /* out of domain. C99 */
#endif

#ifndef ERANGE
#define ERANGE 7 /* Result is too big. C99 */
#endif

#ifndef EILSEQ
#define EILSEQ 8 /* Illegal byte array. C99 */
#endif

#ifndef ENOENT
#define ENOENT 9 /* No such file or directory. */
#endif

#define ERRNO_MAX EINVAL

#ifdef __cplusplus
}
#endif

#endif
