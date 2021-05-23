#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#define MAP_SIZE 5
#define TTC 0
#define SUM 1
#define MAP_ENTRIES 100

struct syscalls_enter_open_args
{
	unsigned long long unused;
	long syscall_nr;
	long filename_ptr;
	long flags;
	long mode;
};

struct
{
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, __u64);
	__uint(max_entries, MAP_SIZE);
} linked_list_map SEC(".maps");

struct
{
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, __u64);
	__uint(max_entries, 2);
} time_to_complete SEC(".maps");



void inc_ptr_content(int *ptr);

static __always_inline void update_time(void *map, __u64 *new_value)
{
	__u32 key = 0, *count_keys;
	__u64 next_key;

	// Retrieve current items count
	count_keys = bpf_map_lookup_elem(map, &key);
	if (count_keys && (*count_keys <= MAP_ENTRIES))
	{

		// Compute next key and insert it
		next_key = (*count_keys) + 1;

		bpf_map_update_elem(map, &next_key, new_value, BPF_ANY);
		bpf_map_update_elem(map, &key, &next_key, BPF_EXIST);
	}
	else
	{
		next_key = 0;
		bpf_map_update_elem(map, &key, &next_key, BPF_NOEXIST);
	}
}

static __always_inline void push(void *map, __u32 key, __u64 val)
{
	__u64 *value;

	value = bpf_map_lookup_elem(map, &key);
	if (value)
		*value = val;
	else
		bpf_map_update_elem(map, &key, &val, BPF_NOEXIST);
}

char msg[] = "End test : ttc = %li, sum = %li\n";
static __always_inline void test()
{
	__u64 begin;
	__u64 end;
	__u64 now;
	__u64 *temp;
	int i = 0;
	int j = 0;
	begin = bpf_ktime_get_ns();

// Fill the map with the current time
#pragma unroll
	for (i = 0; i < MAP_SIZE; i++)
	{
		now = bpf_ktime_get_ns();
		j = i; // For the unroll
		push(&linked_list_map, j, now);
	}

	now = 0;

// Sum the random values of the map
#pragma unroll
	for (i = 0; i < MAP_SIZE; i++)
	{
		j = bpf_get_prandom_u32() % MAP_SIZE;
		temp = bpf_map_lookup_elem(&linked_list_map, &j);
		if (temp)
			now += *((__u64 *)temp);
	}

	end = bpf_ktime_get_ns() - begin;

	update_time(&time_to_complete, &end);
	push(&time_to_complete, SUM, now); // Safe the result for loader.c
									   //   bpf_trace_printk(msg, sizeof(msg), end, now);
}

SEC("tracepoint/syscalls/sys_enter_open")
int trace_enter_open(struct syscalls_enter_open_args *ctx)
{
	test();
	return 0;
}

SEC("tracepoint/syscalls/sys_enter_openat")
int trace_enter_open_at(struct syscalls_enter_open_args *ctx)
{
	test();
	return 0;
}

char _license[] SEC("license") = "GPL";