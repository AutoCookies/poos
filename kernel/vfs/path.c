#include "path.h"
#include "mount.h"
#include "../proc/proc.h"

static int str_eq(const char* a, const char* b) {
    u32 i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return 0; i++; }
    return a[i] == b[i];
}

int path_next_component(const char** path, char* out_name, u32 out_size) {
    u32 i = 0;
    const char* p = *path;
    while (*p == '/') p++;
    if (!*p) { *path = p; return 0; }
    while (*p && *p != '/') {
        if (i + 1 >= out_size) return -1;
        out_name[i++] = *p++;
    }
    out_name[i] = '\0';
    while (*p == '/') p++;
    *path = p;
    return 1;
}

int vfs_resolve(const char* path, struct vnode** out) {
    if (!path || path[0] != '/') return -1;
    struct vnode* jail = proc_current_root();
    struct vnode* cur = jail ? jail : mount_root();
    char comp[64];
    char cur_path[128] = "/";
    const char* p = path;
    int rc;
    if (!cur) return -1;
    vnode_ref(cur);
    for (;;) {
        rc = path_next_component(&p, comp, sizeof(comp));
        if (rc <= 0) break;
        if (str_eq(comp, ".")) continue;
        if (str_eq(comp, "..")) continue;
        if (cur_path[1] != '\0') {
            u32 l = 0; while (cur_path[l]) l++;
            cur_path[l++] = '/'; cur_path[l] = '\0';
        }
        u32 l = 0; while (cur_path[l]) l++;
        for (u32 i = 0; comp[i] && l + 1 < sizeof(cur_path); ++i) cur_path[l++] = comp[i];
        cur_path[l] = '\0';

        struct vnode* m = mount_find(cur_path);
        if (m) {
            vnode_put(cur);
            cur = m;
            vnode_ref(cur);
            continue;
        }
        if (!cur->ops || !cur->ops->lookup) { vnode_put(cur); return -1; }
        struct vnode* next = 0;
        if (cur->ops->lookup(cur, comp, &next) < 0 || !next) { vnode_put(cur); return -1; }
        vnode_put(cur);
        cur = next;
    }
    if (rc < 0) { vnode_put(cur); return -1; }
    *out = cur;
    return 0;
}
