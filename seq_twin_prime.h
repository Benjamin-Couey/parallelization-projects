struct twin_prime {
  long long first;
  long long second;
};

int is_prime(long long num);
struct twin_prime get_nth_twin_prime(int n, int verbose);
