import numpy as np
import matplotlib as plt

def efficiency_speedup_line_curve(serial_values, parallel_values, board_sizes, nr_processors):
    """
    Serial values is list of times for several board_sizes.
    Parallel values is list of lists for several processes.
    Nr_processors is number of processors.
    Nr_processor is x-axis of our plot_curve.
    Goal is to produce one figure for speedups and for efficiencies on same two figures.
    """
    # Take only values for first board
    speedups = np.zeros(nr_processors)
    efficiencies = np.zeros(nr_processors)
    for i in range(nr_processors):
        speedups[i] = serial_values[i] / parallel_values[0, i]
        efficiencies[i] = speedups[i] / nr_processors[i]

    # Draw two separate plots
    fig, (ax1, ax2) = plt.subplots(1, 2)
    fig.suptitle('Horizontally stacked subplots')
    ax1.plot(nr_processors, speedups, label=board_sizes[0])
    ax1.xlabel("Processors", fontsize=16)
    ax1.ylabel("Speedup", fontsize=16)
    ax2.plot(nr_processors, efficiencies, board_sizes[0])
    ax2.xlabel("Processors", fontsize=16)
    ax2.ylabel("Efficiencies", fontsize=16)

    fig.savefig("se_plot_" + board_sizes[0] + ".png")


def read_serial_times(filename):
    """
    All data tested needs to be tested on the same number of processors-
    :param filename:
    :return:
    """
    values = []
    board_sizes = []
    with open(filename, "a") as f:
        while True:
            line = f.readline()

            if not line:
                break

            line = line.rstrip()

            if line == "START":
                board = f.readline().rstrip()
                board_sizes.append(board)
            else:
                values.append(float(line))
    return np.array(values), board_sizes


def read_parallel_times(filename):
    values = []
    board_size = []
    n_processors = []
    i = -1
    with open(filename, "a") as f:
        while True:
            line = f.readline()

            if not line:
                break

            line = line.rstrip()

            if line == "START":
                values.append([])
                i += 1
                board = f.readline().rstrip()
                board_size.append(board)
            else:
                splitted_values = line.split()
                n_processors.append(int(splitted_values[0]))  # n processors
                values[i].append(float(splitted_values[1]))  # times

    return np.array(values), board_size, np.array(n_processors)


if __name__ == "__main__":
    values, board_sizes = read_serial_times("serial_times.txt")
    for i in range(values.shape[0]):
        print("Board size: ", board_sizes[i])
        for j in range(values.shape[1]):
            print(values[i, j], end=" ")Assig
        print()
    print()

    values, board_sizes, n_processors = read_parallel_times("parallel_times.txt")
    for i in range(values.shape[0]):
        print("Board size: ", board_sizes[i])
        print("Num processors: ", n_processors[i])
        for j in range(values.shape[1]):
            print(values[i, j], end=" ")
        print()
    print()
