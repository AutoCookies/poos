#include "../vfs/vnode.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
#include "../tty/tty.h"

void vga_write(const char*);

struct devfs_node { const char* name; enum vnode_type type; };

static int str_eq(const char* a, const char* b) { u32 i=0; while (a[i]&&b[i]) { if (a[i]!=b[i]) return 0; i++; } return a[i]==b[i]; }

static int devfs_read(struct vnode* vn, u32 off, void* buf, u32 len) {
    (void)off;
    struct devfs_node* n = (struct devfs_node*)vn->data;
    if (str_eq(n->name, "tty") || str_eq(n->name, "console")) return tty_read_line(buf, len);
    return 0;
}
static int devfs_write(struct vnode* vn, u32 off, const void* buf, u32 len) {
    (void)off;
    struct devfs_node* n = (struct devfs_node*)vn->data;
    if (str_eq(n->name, "null")) return (int)len;
    if (str_eq(n->name, "console") || str_eq(n->name, "tty")) {
        char tmp[128]; if (len >= sizeof(tmp)) len = sizeof(tmp)-1;
        mem_copy(tmp, buf, len); tmp[len]='\0'; vga_write(tmp); return (int)len;
    }
    return -1;
}
static int devfs_getattr(struct vnode* vn, struct vstat* out) { out->type = vn->type; out->mode = 0; out->size = 0; return 0; }

static struct devfs_node g_console = { "console", VNODE_DEV };
static struct devfs_node g_tty = { "tty", VNODE_DEV };
static struct devfs_node g_null = { "null", VNODE_DEV };
static struct devfs_node g_dev = { "dev", VNODE_DIR };
static struct vnode g_console_vn, g_tty_vn, g_null_vn, g_root_vn;

static int devdir_lookup(struct vnode* dir, const char* name, struct vnode** out) {
    (void)dir;
    if (str_eq(name, "console")) { *out = &g_console_vn; vnode_ref(*out); return 0; }
    if (str_eq(name, "tty")) { *out = &g_tty_vn; vnode_ref(*out); return 0; }
    if (str_eq(name, "null")) { *out = &g_null_vn; vnode_ref(*out); return 0; }
    return -1;
}

static int devdir_readdir(struct vnode* vn, u32* cookie, struct vdirent* out) {
    (void)vn;
    if (*cookie == 0) { out->type = VNODE_DEV; mem_copy(out->name,"console",8); out->name[7]=0; *cookie=1; return 1; }
    if (*cookie == 1) { out->type = VNODE_DEV; mem_copy(out->name,"tty",4); out->name[3]=0; *cookie=2; return 1; }
    if (*cookie == 2) { out->type = VNODE_DEV; mem_copy(out->name,"null",5); out->name[4]=0; *cookie=3; return 1; }
    return 0;
}

static const struct vnode_ops g_devdir_ops = { devdir_lookup, 0, 0, devdir_readdir, devfs_getattr };
static const struct vnode_ops g_devnode_ops = { 0, devfs_read, devfs_write, 0, devfs_getattr };

struct vnode* devfs_root(void) {
    vnode_init(&g_root_vn, VNODE_DIR, &g_devdir_ops, &g_dev);
    vnode_init(&g_console_vn, VNODE_DEV, &g_devnode_ops, &g_console);
    vnode_init(&g_tty_vn, VNODE_DEV, &g_devnode_ops, &g_tty);
    vnode_init(&g_null_vn, VNODE_DEV, &g_devnode_ops, &g_null);
    return &g_root_vn;
}
