#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "gdextension_interface.h"
#include "challenge.h"

#if defined(_WIN32)
#define CG_ENTRY __declspec(dllexport)
#else
#define CG_ENTRY __attribute__((visibility("default")))
#endif

#define CG_NONCE_BUF 512
#define CG_PROP_USAGE_DEFAULT 6u

typedef struct {
    void *opaque[4];
} cg_opaque;

typedef struct {
    cg_ctx *ctx;
} cg_self;

static GDExtensionClassLibraryPtr cg_library;
static cg_opaque cg_class_name;
static cg_opaque cg_parent_name;

static GDExtensionInterfaceClassdbRegisterExtensionClass6 cg_i_register_class;
static GDExtensionInterfaceClassdbRegisterExtensionClassMethod cg_i_register_method;
static GDExtensionInterfaceClassdbConstructObject3 cg_i_construct_object;
static GDExtensionInterfaceObjectSetInstance cg_i_object_set_instance;
static GDExtensionInterfaceMemAlloc cg_i_mem_alloc;
static GDExtensionInterfaceMemFree cg_i_mem_free;
static GDExtensionInterfaceStringNameNewWithLatin1Chars cg_i_sn_new;
static GDExtensionInterfaceStringNewWithUtf8Chars cg_i_string_new;
static GDExtensionInterfaceStringToUtf8Chars cg_i_string_utf8;

static GDExtensionVariantFromTypeConstructorFunc cg_v_from_int;
static GDExtensionVariantFromTypeConstructorFunc cg_v_from_float;
static GDExtensionVariantFromTypeConstructorFunc cg_v_from_bool;
static GDExtensionVariantFromTypeConstructorFunc cg_v_from_string;
static GDExtensionTypeFromVariantConstructorFunc cg_v_to_int;
static GDExtensionTypeFromVariantConstructorFunc cg_v_to_string;
static GDExtensionPtrDestructor cg_d_string;
static GDExtensionPtrDestructor cg_d_string_name;

static cg_ctx *cg_of(GDExtensionClassInstancePtr p)
{
    cg_self *self = (cg_self *)p;
    return self ? self->ctx : NULL;
}

#define CG_DEF_VOID0(fname, call)                                                        \
    static void fname##_ptr(void *u, GDExtensionClassInstancePtr p,                      \
                            const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)      \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        (void)u; (void)a; (void)r;                                                       \
        call;                                                                            \
    }                                                                                    \
    static void fname##_call(void *u, GDExtensionClassInstancePtr p,                     \
                             const GDExtensionConstVariantPtr *a, GDExtensionInt n,      \
                             GDExtensionVariantPtr r, GDExtensionCallError *e)           \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        (void)u; (void)a; (void)n; (void)r;                                              \
        e->error = GDEXTENSION_CALL_OK;                                                  \
        call;                                                                            \
    }

#define CG_DEF_INT0(fname, call)                                                         \
    static void fname##_ptr(void *u, GDExtensionClassInstancePtr p,                      \
                            const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)      \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        (void)u; (void)a;                                                                \
        *(int64_t *)r = (int64_t)(call);                                                 \
    }                                                                                    \
    static void fname##_call(void *u, GDExtensionClassInstancePtr p,                     \
                             const GDExtensionConstVariantPtr *a, GDExtensionInt n,      \
                             GDExtensionVariantPtr r, GDExtensionCallError *e)           \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        int64_t v;                                                                       \
        (void)u; (void)a; (void)n;                                                       \
        v = (int64_t)(call);                                                             \
        cg_v_from_int(r, &v);                                                            \
        e->error = GDEXTENSION_CALL_OK;                                                  \
    }

#define CG_DEF_BOOL0(fname, call)                                                        \
    static void fname##_ptr(void *u, GDExtensionClassInstancePtr p,                      \
                            const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)      \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        (void)u; (void)a;                                                                \
        *(uint8_t *)r = (call) ? 1u : 0u;                                                \
    }                                                                                    \
    static void fname##_call(void *u, GDExtensionClassInstancePtr p,                     \
                             const GDExtensionConstVariantPtr *a, GDExtensionInt n,      \
                             GDExtensionVariantPtr r, GDExtensionCallError *e)           \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        GDExtensionBool v;                                                               \
        (void)u; (void)a; (void)n;                                                       \
        v = (GDExtensionBool)((call) ? 1 : 0);                                           \
        cg_v_from_bool(r, &v);                                                           \
        e->error = GDEXTENSION_CALL_OK;                                                  \
    }

#define CG_DEF_FLOAT0(fname, call)                                                       \
    static void fname##_ptr(void *u, GDExtensionClassInstancePtr p,                      \
                            const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)      \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        (void)u; (void)a;                                                                \
        *(double *)r = (double)(call);                                                   \
    }                                                                                    \
    static void fname##_call(void *u, GDExtensionClassInstancePtr p,                     \
                             const GDExtensionConstVariantPtr *a, GDExtensionInt n,      \
                             GDExtensionVariantPtr r, GDExtensionCallError *e)           \
    {                                                                                    \
        cg_ctx *c = cg_of(p);                                                            \
        double v;                                                                        \
        (void)u; (void)a; (void)n;                                                       \
        v = (double)(call);                                                              \
        cg_v_from_float(r, &v);                                                          \
        e->error = GDEXTENSION_CALL_OK;                                                  \
    }

static void cg_impl_create(cg_self *self, int64_t game_id)
{
    if (!self)
        return;
    if (self->ctx) {
        cg_destroy(self->ctx);
        self->ctx = NULL;
    }
    self->ctx = cg_create((unsigned int)(uint32_t)game_id);
}

static void cg_m_create_ptr(void *u, GDExtensionClassInstancePtr p,
                            const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)
{
    (void)u; (void)r;
    cg_impl_create((cg_self *)p, *(const int64_t *)a[0]);
}

static void cg_m_create_call(void *u, GDExtensionClassInstancePtr p,
                             const GDExtensionConstVariantPtr *a, GDExtensionInt n,
                             GDExtensionVariantPtr r, GDExtensionCallError *e)
{
    int64_t game_id = 0;
    (void)u; (void)r;
    if (n < 1) {
        e->error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        e->argument = 1;
        e->expected = 1;
        return;
    }
    cg_v_to_int(&game_id, (GDExtensionVariantPtr)a[0]);
    cg_impl_create((cg_self *)p, game_id);
    e->error = GDEXTENSION_CALL_OK;
}

static void cg_m_add_score_ptr(void *u, GDExtensionClassInstancePtr p,
                               const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)
{
    (void)u; (void)r;
    cg_add_score(cg_of(p), (int)*(const int64_t *)a[0]);
}

static void cg_m_add_score_call(void *u, GDExtensionClassInstancePtr p,
                                const GDExtensionConstVariantPtr *a, GDExtensionInt n,
                                GDExtensionVariantPtr r, GDExtensionCallError *e)
{
    int64_t delta = 0;
    (void)u; (void)r;
    if (n < 1) {
        e->error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        e->argument = 1;
        e->expected = 1;
        return;
    }
    cg_v_to_int(&delta, (GDExtensionVariantPtr)a[0]);
    cg_add_score(cg_of(p), (int)delta);
    e->error = GDEXTENSION_CALL_OK;
}

static void cg_m_flow_ptr(void *u, GDExtensionClassInstancePtr p,
                          const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)
{
    (void)u; (void)r;
    cg_motion(cg_of(p), (int)*(const int64_t *)a[0]);
}

static void cg_m_flow_call(void *u, GDExtensionClassInstancePtr p,
                           const GDExtensionConstVariantPtr *a, GDExtensionInt n,
                           GDExtensionVariantPtr r, GDExtensionCallError *e)
{
    int64_t step = 0;
    (void)u; (void)r;
    if (n < 1) {
        e->error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        e->argument = 1;
        e->expected = 1;
        return;
    }
    cg_v_to_int(&step, (GDExtensionVariantPtr)a[0]);
    cg_motion(cg_of(p), (int)step);
    e->error = GDEXTENSION_CALL_OK;
}

static const char *cg_impl_receipt(cg_ctx *c, char *buf, size_t buf_len)
{
    if (!c || cg_receipt(c, buf, buf_len) != CG_OK)
        return "";
    return buf;
}

static void cg_m_receipt_ptr(void *u, GDExtensionClassInstancePtr p,
                             const GDExtensionConstTypePtr *a, GDExtensionTypePtr r)
{
    char buf[CG_REWARD_MAX];
    const char *s;
    (void)u; (void)a;
    s = cg_impl_receipt(cg_of(p), buf, sizeof(buf));
    cg_d_string(r);
    cg_i_string_new(r, s);
    memset(buf, 0, sizeof(buf));
}

static void cg_m_receipt_call(void *u, GDExtensionClassInstancePtr p,
                              const GDExtensionConstVariantPtr *a, GDExtensionInt n,
                              GDExtensionVariantPtr r, GDExtensionCallError *e)
{
    char buf[CG_REWARD_MAX];
    cg_opaque tmp;
    const char *s;
    (void)u; (void)a; (void)n;
    s = cg_impl_receipt(cg_of(p), buf, sizeof(buf));
    cg_i_string_new(&tmp, s);
    cg_v_from_string(r, &tmp);
    cg_d_string(&tmp);
    memset(buf, 0, sizeof(buf));
    e->error = GDEXTENSION_CALL_OK;
}

CG_DEF_INT0(cg_m_get_score, cg_get_score(c))
CG_DEF_VOID0(cg_m_take_hit, cg_take_hit(c))
CG_DEF_VOID0(cg_m_collect_bottle, cg_collect_bottle(c))
CG_DEF_INT0(cg_m_get_bottles, cg_get_bottles(c))
CG_DEF_BOOL0(cg_m_is_caught, cg_is_caught(c))
CG_DEF_VOID0(cg_m_reset_run, cg_reset_run(c))
CG_DEF_FLOAT0(cg_m_drift, cg_get_lift(c))
CG_DEF_INT0(cg_m_mode, cg_motion_mode())
CG_DEF_INT0(cg_m_seal, cg_phase_a(c))
CG_DEF_INT0(cg_m_fold, cg_phase_b(c))

static GDExtensionObjectPtr cg_class_create(void *userdata, GDExtensionBool notify_postinitialize)
{
    GDExtensionObjectPtr obj;
    cg_self *self;

    (void)userdata;
    (void)notify_postinitialize;

    obj = cg_i_construct_object(&cg_parent_name);
    if (!obj)
        return NULL;

    self = (cg_self *)cg_i_mem_alloc(sizeof(cg_self));
    if (!self)
        return obj;
    self->ctx = NULL;

    cg_i_object_set_instance(obj, &cg_class_name, self);
    return obj;
}

static void cg_class_free(void *userdata, GDExtensionClassInstancePtr instance)
{
    cg_self *self = (cg_self *)instance;

    (void)userdata;
    if (!self)
        return;
    if (self->ctx) {
        cg_destroy(self->ctx);
        self->ctx = NULL;
    }
    cg_i_mem_free(self);
}

static GDExtensionClassCallVirtual cg_class_get_virtual(void *userdata,
                                                        GDExtensionConstStringNamePtr name,
                                                        uint32_t hash)
{
    (void)userdata;
    (void)name;
    (void)hash;
    return NULL;
}

static void cg_bind(const char *name,
                    GDExtensionClassMethodCall call_func,
                    GDExtensionClassMethodPtrCall ptrcall_func,
                    GDExtensionVariantType ret_type,
                    GDExtensionVariantType arg_type,
                    const char *arg_name)
{
    GDExtensionClassMethodInfo mi;
    GDExtensionPropertyInfo ret_info;
    GDExtensionPropertyInfo arg_info;
    GDExtensionClassMethodArgumentMetadata arg_meta = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
    cg_opaque m_name;
    cg_opaque a_name;
    cg_opaque empty_sn;
    cg_opaque empty_str;

    cg_i_sn_new(&m_name, name, 0);
    cg_i_sn_new(&a_name, arg_name ? arg_name : "", 0);
    cg_i_sn_new(&empty_sn, "", 0);
    cg_i_string_new(&empty_str, "");

    memset(&mi, 0, sizeof(mi));
    memset(&ret_info, 0, sizeof(ret_info));

    ret_info.type = ret_type;
    ret_info.name = &empty_sn;
    ret_info.class_name = &empty_sn;
    ret_info.hint = 0;
    ret_info.hint_string = &empty_str;
    ret_info.usage = CG_PROP_USAGE_DEFAULT;

    arg_info = ret_info;
    arg_info.type = arg_type;
    arg_info.name = &a_name;

    mi.name = &m_name;
    mi.method_userdata = NULL;
    mi.call_func = call_func;
    mi.ptrcall_func = ptrcall_func;
    mi.method_flags = GDEXTENSION_METHOD_FLAGS_DEFAULT;
    mi.has_return_value = (GDExtensionBool)(ret_type != GDEXTENSION_VARIANT_TYPE_NIL);
    mi.return_value_info = &ret_info;
    mi.return_value_metadata = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
    mi.argument_count = arg_name ? 1u : 0u;
    mi.arguments_info = &arg_info;
    mi.arguments_metadata = &arg_meta;
    mi.default_argument_count = 0;
    mi.default_arguments = NULL;

    cg_i_register_method(cg_library, &cg_class_name, &mi);

    cg_d_string_name(&m_name);
    cg_d_string_name(&a_name);
    cg_d_string_name(&empty_sn);
    cg_d_string(&empty_str);
}

static void cg_register_class(void)
{
    GDExtensionClassCreationInfo6 ci;

    memset(&ci, 0, sizeof(ci));
    ci.is_virtual = 0;
    ci.is_abstract = 0;
    ci.is_exposed = 1;
    ci.is_runtime = 0;
    ci.create_instance_func = cg_class_create;
    ci.free_instance_func = cg_class_free;
    ci.get_virtual_func = cg_class_get_virtual;
    ci.class_userdata = NULL;

    cg_i_sn_new(&cg_class_name, "CGChallenge", 0);
    cg_i_sn_new(&cg_parent_name, "RefCounted", 0);

    cg_i_register_class(cg_library, &cg_class_name, &cg_parent_name, &ci);

    cg_bind("create", cg_m_create_call, cg_m_create_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_INT, "game_id");
    cg_bind("add_score", cg_m_add_score_call, cg_m_add_score_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_INT, "delta");
    cg_bind("get_score", cg_m_get_score_call, cg_m_get_score_ptr,
            GDEXTENSION_VARIANT_TYPE_INT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("take_hit", cg_m_take_hit_call, cg_m_take_hit_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("collect_bottle", cg_m_collect_bottle_call, cg_m_collect_bottle_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("get_bottles", cg_m_get_bottles_call, cg_m_get_bottles_ptr,
            GDEXTENSION_VARIANT_TYPE_INT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("is_caught", cg_m_is_caught_call, cg_m_is_caught_ptr,
            GDEXTENSION_VARIANT_TYPE_BOOL, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("reset_run", cg_m_reset_run_call, cg_m_reset_run_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("drift", cg_m_drift_call, cg_m_drift_ptr,
            GDEXTENSION_VARIANT_TYPE_FLOAT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("mode", cg_m_mode_call, cg_m_mode_ptr,
            GDEXTENSION_VARIANT_TYPE_INT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("flow", cg_m_flow_call, cg_m_flow_ptr,
            GDEXTENSION_VARIANT_TYPE_NIL, GDEXTENSION_VARIANT_TYPE_INT, "step");
    cg_bind("seal", cg_m_seal_call, cg_m_seal_ptr,
            GDEXTENSION_VARIANT_TYPE_INT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("fold", cg_m_fold_call, cg_m_fold_ptr,
            GDEXTENSION_VARIANT_TYPE_INT, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
    cg_bind("open", cg_m_receipt_call, cg_m_receipt_ptr,
            GDEXTENSION_VARIANT_TYPE_STRING, GDEXTENSION_VARIANT_TYPE_NIL, NULL);
}

static void cg_initialize(void *userdata, GDExtensionInitializationLevel level)
{
    (void)userdata;
    if (level != GDEXTENSION_INITIALIZATION_SCENE)
        return;
    cg_register_class();
}

static void cg_deinitialize(void *userdata, GDExtensionInitializationLevel level)
{
    (void)userdata;
    if (level != GDEXTENSION_INITIALIZATION_SCENE)
        return;
    cg_d_string_name(&cg_class_name);
    cg_d_string_name(&cg_parent_name);
}

static void *cg_proc(GDExtensionInterfaceGetProcAddress get, const char *name)
{
    return (void *)get(name);
}

static int cg_load_interface(GDExtensionInterfaceGetProcAddress get)
{
    GDExtensionInterfaceGetVariantFromTypeConstructor from_type;
    GDExtensionInterfaceGetVariantToTypeConstructor to_type;
    GDExtensionInterfaceVariantGetPtrDestructor get_destructor;

    cg_i_register_class = (GDExtensionInterfaceClassdbRegisterExtensionClass6)
        cg_proc(get, "classdb_register_extension_class6");
    cg_i_register_method = (GDExtensionInterfaceClassdbRegisterExtensionClassMethod)
        cg_proc(get, "classdb_register_extension_class_method");
    cg_i_construct_object = (GDExtensionInterfaceClassdbConstructObject3)
        cg_proc(get, "classdb_construct_object3");
    cg_i_object_set_instance = (GDExtensionInterfaceObjectSetInstance)
        cg_proc(get, "object_set_instance");
    cg_i_mem_alloc = (GDExtensionInterfaceMemAlloc)cg_proc(get, "mem_alloc");
    cg_i_mem_free = (GDExtensionInterfaceMemFree)cg_proc(get, "mem_free");
    cg_i_sn_new = (GDExtensionInterfaceStringNameNewWithLatin1Chars)
        cg_proc(get, "string_name_new_with_latin1_chars");
    cg_i_string_new = (GDExtensionInterfaceStringNewWithUtf8Chars)
        cg_proc(get, "string_new_with_utf8_chars");
    cg_i_string_utf8 = (GDExtensionInterfaceStringToUtf8Chars)
        cg_proc(get, "string_to_utf8_chars");

    from_type = (GDExtensionInterfaceGetVariantFromTypeConstructor)
        cg_proc(get, "get_variant_from_type_constructor");
    to_type = (GDExtensionInterfaceGetVariantToTypeConstructor)
        cg_proc(get, "get_variant_to_type_constructor");
    get_destructor = (GDExtensionInterfaceVariantGetPtrDestructor)
        cg_proc(get, "variant_get_ptr_destructor");

    if (!cg_i_register_class || !cg_i_register_method || !cg_i_construct_object ||
        !cg_i_object_set_instance || !cg_i_mem_alloc || !cg_i_mem_free ||
        !cg_i_sn_new || !cg_i_string_new || !cg_i_string_utf8 ||
        !from_type || !to_type || !get_destructor)
        return 0;

    cg_v_from_int = from_type(GDEXTENSION_VARIANT_TYPE_INT);
    cg_v_from_float = from_type(GDEXTENSION_VARIANT_TYPE_FLOAT);
    cg_v_from_bool = from_type(GDEXTENSION_VARIANT_TYPE_BOOL);
    cg_v_from_string = from_type(GDEXTENSION_VARIANT_TYPE_STRING);
    cg_v_to_int = to_type(GDEXTENSION_VARIANT_TYPE_INT);
    cg_v_to_string = to_type(GDEXTENSION_VARIANT_TYPE_STRING);
    cg_d_string = get_destructor(GDEXTENSION_VARIANT_TYPE_STRING);
    cg_d_string_name = get_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);

    if (!cg_v_from_int || !cg_v_from_float || !cg_v_from_bool || !cg_v_from_string ||
        !cg_v_to_int || !cg_v_to_string || !cg_d_string || !cg_d_string_name)
        return 0;

    return 1;
}

CG_ENTRY GDExtensionBool cg_gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                             GDExtensionClassLibraryPtr p_library,
                                             GDExtensionInitialization *r_initialization)
{
    if (!cg_load_interface(p_get_proc_address))
        return 0;

    cg_library = p_library;

    r_initialization->minimum_initialization_level = GDEXTENSION_INITIALIZATION_SCENE;
    r_initialization->userdata = NULL;
    r_initialization->initialize = cg_initialize;
    r_initialization->deinitialize = cg_deinitialize;

    return 1;
}
