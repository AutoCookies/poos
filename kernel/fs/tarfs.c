#include "tarfs.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

struct tar_header {
    char name[100]; char mode[8]; char uid[8]; char gid[8]; char size[12]; char mtime[12];
    char chksum[8]; char typeflag; char linkname[100]; char magic[6]; char version[2];
    char uname[32]; char gname[32]; char devmajor[8]; char devminor[8]; char prefix[155];
};

struct tarfs_mount { const u8* base; u32 size; struct vnode root; };
struct tarfs_node { struct tarfs_mount* mnt; u32 hdr_off; int is_dir; char path[128]; struct vnode vn; };

static u32 oct2u(const char* s, u32 n){ u32 v=0; for(u32 i=0;i<n&&s[i];++i){ if(s[i]<'0'||s[i]>'7') break; v=(v<<3)+(u32)(s[i]-'0'); } return v; }
static u32 str_len(const char* s){u32 i=0;while(s[i])i++;return i;}
static int str_eq(const char*a,const char*b){u32 i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return 0;i++;}return a[i]==b[i];}
static int str_prefix(const char* s, const char* p){u32 i=0; while(p[i]){ if(s[i]!=p[i]) return 0; i++; } return 1; }

static int name_is_empty(const char* n){ for (u32 i=0;i<100;i++) if (n[i]) return 0; return 1; }

static const struct tar_header* hdr_at(struct tarfs_mount* m, u32 off) { return (const struct tar_header*)(m->base + off); }
static u32 next_hdr_off(struct tarfs_mount* m, u32 off) {
    const struct tar_header* h = hdr_at(m, off);
    u32 sz = oct2u(h->size, 11);
    return off + 512U + ((sz + 511U) & ~511U);
}

static int tar_lookup_impl(struct tarfs_mount* m, const char* parent, const char* name, struct vnode** out) {
    u32 plen = str_len(parent);
    u32 off = 0;
    while (off + 512 <= m->size) {
        const struct tar_header* h = hdr_at(m, off);
        if (name_is_empty(h->name)) break;
        char full[128]; u32 fi=0;
        if (h->prefix[0]) {
            for (u32 i=0; h->prefix[i] && fi+1<sizeof(full); ++i) full[fi++]=h->prefix[i];
            if (fi+1<sizeof(full)) full[fi++]='/';
        }
        for (u32 i=0; h->name[i] && fi+1<sizeof(full); ++i) full[fi++]=h->name[i];
        full[fi]='\0';
        if (plen && !str_prefix(full, parent)) { off = next_hdr_off(m, off); continue; }
        const char* rest = full + plen;
        if (plen && rest[0]=='/') rest++;
        u32 ri=0; char comp[64];
        while (rest[ri] && rest[ri] != '/' && ri+1 < sizeof(comp)) { comp[ri]=rest[ri]; ri++; }
        comp[ri]='\0';
        if (str_eq(comp, name) && (rest[ri]=='\0' || rest[ri]=='/')) {
            struct tarfs_node* n = (struct tarfs_node*)kmalloc(sizeof(*n), 8);
            if (!n) return -1;
            mem_set(n, 0, sizeof(*n));
            n->mnt = m; n->hdr_off = off;
            n->is_dir = (h->typeflag == '5') || (rest[ri]=='/');
            for (u32 i=0; full[i] && i+1<sizeof(n->path); ++i) n->path[i] = full[i];
            vnode_init(&n->vn, n->is_dir ? VNODE_DIR : VNODE_REG, 0, n);
            *out = &n->vn;
            return 0;
        }
        off = next_hdr_off(m, off);
    }
    return -1;
}

static int tar_lookup(struct vnode* dir, const char* name, struct vnode** out);
static int tar_read(struct vnode* vn, u32 off, void* buf, u32 len);
static int tar_readdir(struct vnode* vn, u32* cookie, struct vdirent* out);
static int tar_getattr(struct vnode* vn, struct vstat* out);

static const struct vnode_ops g_tar_ops = {
    .lookup = tar_lookup,
    .read = tar_read,
    .write = 0,
    .readdir = tar_readdir,
    .getattr = tar_getattr
};

static void bind_ops(struct vnode* vn) { vn->ops = &g_tar_ops; }

static int tar_lookup(struct vnode* dir, const char* name, struct vnode** out) {
    struct tarfs_node* d = (struct tarfs_node*)dir->data;
    return tar_lookup_impl(d->mnt, d->path, name, out);
}

static int tar_read(struct vnode* vn, u32 off, void* buf, u32 len) {
    struct tarfs_node* n = (struct tarfs_node*)vn->data;
    const struct tar_header* h = hdr_at(n->mnt, n->hdr_off);
    u32 sz = oct2u(h->size, 11);
    if (off >= sz) return 0;
    if (off + len > sz) len = sz - off;
    mem_copy(buf, (const void*)((u32)(h + 1) + off), len);
    return (int)len;
}

static int tar_readdir(struct vnode* vn, u32* cookie, struct vdirent* out) {
    struct tarfs_node* d = (struct tarfs_node*)vn->data;
    u32 idx = 0, off = 0;
    while (off + 512 <= d->mnt->size) {
        const struct tar_header* h = hdr_at(d->mnt, off);
        if (name_is_empty(h->name)) break;
        char full[128]; u32 fi=0;
        if (h->prefix[0]) {
            for (u32 i=0; h->prefix[i] && fi+1<sizeof(full); ++i) full[fi++]=h->prefix[i];
            if (fi+1<sizeof(full)) full[fi++]='/';
        }
        for (u32 i=0; h->name[i] && fi+1<sizeof(full); ++i) full[fi++]=h->name[i];
        full[fi]='\0';
        if (d->path[0] && !str_prefix(full, d->path)) { off = next_hdr_off(d->mnt, off); continue; }
        const char* rest = full + str_len(d->path);
        if (d->path[0] && rest[0]=='/') rest++;
        if (!rest[0]) { off = next_hdr_off(d->mnt, off); continue; }
        u32 ri=0; while (rest[ri] && rest[ri] != '/' && ri+1 < sizeof(out->name)) { out->name[ri]=rest[ri]; ri++; }
        out->name[ri]='\0';
        if (idx++ < *cookie) { off = next_hdr_off(d->mnt, off); continue; }
        *cookie = idx;
        out->type = (rest[ri]=='/' || h->typeflag=='5') ? VNODE_DIR : VNODE_REG;
        return 1;
    }
    return 0;
}

static int tar_getattr(struct vnode* vn, struct vstat* out) {
    struct tarfs_node* n = (struct tarfs_node*)vn->data;
    const struct tar_header* h = hdr_at(n->mnt, n->hdr_off);
    out->type = n->is_dir ? VNODE_DIR : VNODE_REG;
    out->mode = oct2u(h->mode, 7); out->uid=vn->uid; out->gid=vn->gid;
    out->size = n->is_dir ? 0 : oct2u(h->size, 11);
    return 0;
}

int tarfs_mount_root(const u8* tar, u32 size, struct vnode** out_root) {
    struct tarfs_mount* m = (struct tarfs_mount*)kmalloc(sizeof(*m), 8);
    struct tarfs_node* n = (struct tarfs_node*)kmalloc(sizeof(*n), 8);
    if (!m || !n) return -1;
    m->base = tar; m->size = size;
    mem_set(n, 0, sizeof(*n));
    n->mnt = m; n->hdr_off = 0; n->is_dir = 1; n->path[0] = '\0';
    vnode_init(&m->root, VNODE_DIR, &g_tar_ops, n);
    bind_ops(&m->root);
    *out_root = &m->root;
    return 0;
}
