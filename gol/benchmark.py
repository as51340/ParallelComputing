import numpy as np
import matplotlib.pyplot as plt


def efficiency_speedup_line_curve(serial_values, parallel_values, board_sizes, nr_processors):
    """
    Serial values is list of times for several board_sizes.
    Parallel values is list of lists for several processes.
    Nr_processors is number of processors.
    Nr_processor is x-axis of our plot_curve.
    Goal is to produce one figure for speedups and for efficiencies on same two figures.
    """
    # Take only values for first board
    fig, (ax1, ax2) = plt.subplots(2, 1)
    fig.suptitle('Game of life, speedup and efficiency, 1000 iterations')
    for i in range(parallel_values.shape[0]):
        speedups = np.zeros(parallel_values.shape[1])
        efficiencies = np.zeros(parallel_values.shape[1])
        print(board_sizes[i])
        for j in range(parallel_values.shape[1]):
            speedups[j] = serial_values[i] / parallel_values[i, j]
            efficiencies[j] = speedups[j] / nr_processors[i, j]
            print("Speedup: ", speedups[j], "Efficiency: ", efficiencies[j])
            print("Serial time: ", serial_values[i], "Parallel time: ", parallel_values[i, j])
            print("Nr_processors: ", nr_processors[i, j])
            print()

        ax1.plot(nr_processors[i, :], speedups, label=board_sizes[i])
        ax1.scatter(nr_processors[i, :], speedups)
        ax2.plot(nr_processors[i, :], efficiencies, label=board_sizes[i])
        ax2.scatter(nr_processors[i, :], efficiencies)

    ax1.set_xlabel("Processors", fontsize=16)
    ax1.set_ylabel("Speedup", fontsize=16)
    ax2.set_xlabel("Processors", fontsize=16)
    ax2.set_ylabel("Efficiency", fontsize=16)
    ax2.legend()
    ax1.legend()

    fig.set_size_inches(8.5, 4.5, forward=True)

    fig.savefig("gol_speed_eff_small" + ".png")


def read_serial_times(filename):
    """
    All data tested needs to be tested on the same number of processors-
    :param filename:
    :return:
    """
    values = []
    board_sizes = []
    with open(filename, "r") as f:
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
    with open(filename, "r") as f:
        while True:
            line = f.readline()

            if not line:
                break

            line = line.rstrip()

            if line == "START":
                values.append([])
                n_processors.append([])
                i += 1
                board = f.readline().rstrip()
                board_size.append(board)
            else:
                splitted_values = line.split()
                n_processors[i].append(int(splitted_values[0]))  # n processors
                values[i].append(float(splitted_values[1]))  # times

    return np.array(values), board_size, np.array(n_processors)


if __name__ == "__main__":
    serial_values, board_sizes_serial = read_serial_times("./serial_times.txt")
    for i in range(serial_values.shape[0]):
        print("Board size: ", board_sizes_serial[i])
        print("Serial time: ", serial_values[i])
    print()

    parallel_values, board_sizes_parallel, n_processors = read_parallel_times("parallel_times.txt")
    for i in range(parallel_values.shape[0]):
        print("Board size: ", board_sizes_parallel[i])
        for j in range(parallel_values.shape[1]):
            print(n_processors[i, j], parallel_values[i, j])
        print()
    print()

    efficiency_speedup_line_curve(serial_values, parallel_values, board_sizes_parallel, n_processors)
