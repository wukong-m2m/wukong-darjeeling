package javax.rtcbench;

public class CoreResults {
	/* inputs */
	public short	seed1;		/* Initializing seed */
	public short	seed2;		/* Initializing seed */
	public short	seed3;		/* Initializing seed */
	public int size;		/* Size of the data */
	public int iterations;		/* Number of iterations to execute */
	public int execs;		/* Bitmask of operations to execute */
	// struct list_head_s *list;
	// For Matrix benchmark
	CoreMatrix.MatParams mat;
	// For State benchmark
	byte[] statememblock3;
	// For List benchmark
	short list_CoreListJoinB; // for the B implementation, the pointer to the list is simply the index into the static data array in CoreListJoinB that will be returned by core_list_init.
	/* outputs */
	short	crc;
	short	crclist;
	short	crcmatrix;
	short	crcstate;
	public short	err;
	/* ultithread specific */
	// core_portable port;
}

// /* Helper structure to hold results */
// typedef struct RESULTS_S {
// 	/* inputs */
// 	ee_s16	seed1;		/* Initializing seed */
// 	ee_s16	seed2;		/* Initializing seed */
// 	ee_s16	seed3;		/* Initializing seed */
// 	void	*memblock[4];	/* Pointer to safe memory location */
// 	ee_u32	size;		/* Size of the data */
// 	ee_u32 iterations;		/* Number of iterations to execute */
// 	ee_u32	execs;		/* Bitmask of operations to execute */
// 	struct list_head_s *list;
// 	mat_params mat;
// 	/* outputs */
// 	ee_u16	crc;
// 	ee_u16	crclist;
// 	ee_u16	crcmatrix;
// 	ee_u16	crcstate;
// 	ee_s16	err;
// 	/* ultithread specific */
// 	core_portable port;
// } core_results;