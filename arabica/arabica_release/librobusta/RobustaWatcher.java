class RobustaWatcher {
	public static int sand=0;

	public static void updateSand(int sandUpdate) {
		sand += sandUpdate;
	}

	public static int getSand() {
		return sand;
	}

	public static void resetSand() {
		sand = 0;
	}
}
