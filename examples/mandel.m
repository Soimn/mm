main :: proc
{
	SCREEN_WIDTH  :: 100;
	SCREEN_HEIGHT :: 100;
	screen: [SCREEN_WIDTH * SCREEN_HEIGHT]u8;

	while (y := 0; y < SCREEN_HEIGHT; y += 1)
	{
		while (x := 0; x < SCREEN_WIDTH; x += 1)
		{
			max_iteration_count := 1000;

			z_r, z_i            := x, y;
			iterations_survived := 0;
			while (; iterations_survived < max_iteration_count; iterations_survived += 1)
			{
				if (z_r <= -SCREEN_WIDTH/2  || z_r >= SCREEN_WIDTH/2 || z_i <= -SCREEN_HEIGHT/2 || z_i >= SCREEN_HEIGHT/2) break;
				else
				{
					// (a + bi)^2 = a^2 + 2abi + b^2i^2 = a^2 - b^2 + 2abi
					z_r, z_i = z_r*z_r - z_i*z_i, 2*z_r*z_i;
				}
			}

			val_map := cast([]u8, "&%8BO#=~;:. ");

			index: uint;
			if (iterations_survived < len(val_map)) index = iterations_survived;
			else                                    index = len(val_map) - 1;

			screen[y*SCREEN_WIDTH + SCREEN_HEIGHT] = val_map[index];
		}
	}
}