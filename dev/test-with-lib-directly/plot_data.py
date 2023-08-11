import sys
import numpy as np
import matplotlib.pyplot as plt

def read_data(filename):
    times = []
    successes = []
    with open(filename, 'r') as file:
        for line in file:
            time_str, success_str = line.strip().split(',')
            times.append(float(time_str))
            successes.append(bool(int(success_str)))
    return np.array(times), np.array(successes)

def plot_graph(times, successes, title):
    plt.scatter(times[successes], [1] * np.sum(successes), color='green', label='Success', marker='o')
    plt.scatter(times[~successes], [1] * np.sum(~successes), color='red', label='Failure', marker='x')
    plt.yticks([])
    plt.xlabel('Response Time (ms)')
    plt.legend()
    plt.title('Scale Response Time and Success/Failure - ' + title)
    plt.grid(True)
    plt.show()

def plot_histogram(times, title):
    plt.hist(times, bins=15, edgecolor='black', alpha=0.7)
    plt.xlabel('Execution Time (seconds)')
    plt.ylabel('Frequency')
    plt.title('Histogram of Execution Times - ' + title)
    plt.grid(True)
    plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: python plot_data.py <filename>")
        sys.exit(1)

    filename = sys.argv[1]
    times, successes = read_data(filename)
    plot_graph(times, successes, filename)

    success_times = times[successes]
    plot_histogram(success_times, filename)
