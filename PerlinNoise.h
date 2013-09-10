//********************
//** PERLIN NOISE FUNCTIONS
//********************

class PerlinNoise {
  static double m_dCosine[50 + 2 * 99999 + 50];
public:
  PerlinNoise(); // Initialize cosine table
  static double Noise(int x, int y);
  static double SmoothNoise(int x, int y);
  static double Cosine_Interpolate(double a, double b, double x);
  static double InterpolatedNoise(double x, double y);
  static double PerlinNoise_2D(double x, double y, int nOctaves, double dPersistence);
};