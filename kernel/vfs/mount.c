#include "mount.h"
#include "vnode.h"

static struct mount g_mounts[4];
static u32 g_mount_count;

void mount_init(void) { g_mount_count = 0; }

int mount_attach(const char* path, struct vnode* root) {
    if (g_mount_count >= 4) return -1;
    g_mounts[g_mount_count].path = path;
    g_mounts[g_mount_count].root = root;
    vnode_ref(root);
    g_mount_count++;
    return 0;
}

struct vnode* mount_root(void) { return g_mount_count ? g_mounts[0].root : 0; }

static int str_eq(const char* a, const char* b) {
    u32 i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return 0; i++; }
    return a[i] == b[i];
}

struct vnode* mount_find(const char* path) {
    for (u32 i = 0; i < g_mount_count; ++i) {
        if (str_eq(path, g_mounts[i].path)) return g_mounts[i].root;
    }
    return 0;
}
