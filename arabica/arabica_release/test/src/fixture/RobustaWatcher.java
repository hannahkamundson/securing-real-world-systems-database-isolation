package fixture;

public class RobustaWatcher
{
	public static int update_count=0;

	public static int sand;

	public static void updateSand(int sandUpdate)
	{
		update_count++;
		sand += sandUpdate;
	}

	public static int getSand()
	{
		return sand;
		//return sand-(10*update_count);
	}

	public static int getUpdateCount()
	{
		return update_count;
	}

	public static void resetSand()
	{
		sand = 0;
		update_count=0;
	}

	public static void resetUpdateCount()
	{
		update_count = 0;
	}
}
