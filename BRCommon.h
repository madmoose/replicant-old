#ifndef BR_COMMON_H
#define BR_COMMON_H

#define BOOL int
#define YES  1
#define NO   0

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define DISTANCE(b, e) ((e) - (b))

#define cleanup_if(x) do { if (x) { printf("%s %s:%d:\tcleanup_if failed: " #x "\n", __FILE__, __func__, __LINE__); goto cleanup; } } while (0)

#define cleanup_if_not(x) do { if (!(x)) { printf("%s %s:%d:\tcleanup_if_not failed: " #x "\n", __FILE__, __func__, __LINE__); goto cleanup; } } while (0)

#endif
