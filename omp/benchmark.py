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


def plt_normal(parallel_values, nr_cores, board_sizes, plt_title, filename):
    """
    Plots speedup curve from given values
    :param parallel_values: 
    :param n_cores: 
    :param board_sizes: 
    :param plt_title: 
    :param filename: 
    :return: 
    """
    fig, ax1 = plt.subplots(1, 1)

    fig.suptitle(plt_title)
    for i in range(parallel_values.shape[0]):
        seq = parallel_values[i, 0]
        speedups = np.zeros(parallel_values.shape[1])
        for j in range(parallel_values.shape[1]):
            speedups[j] = seq / parallel_values[i, j]
        ax1.plot(nr_cores[i, :], speedups, label=board_sizes[i])
        ax1.scatter(nr_cores[i, :], speedups)

    ax1.set_xlabel("Cores")
    ax1.set_ylabel("Speedup")
    ax1.legend()
    fig.set_size_inches(8.5, 4.5, forward=True)
    fig.savefig(filename)




def read_parallel_times(filename):
    values = []
    board_size = []
    n_processors = []
    i = -1
    plt_title = None
    with open(filename, "r") as f:
        plt_title = f.readline().rstrip()
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

    return np.array(values), board_size, np.ar3

if __name__ == "__main__":
    parallel_values, board_sizes_parallel, nr_cores, plt_title = read_parallel_times("diagonal_results.txt")
    print(f"Plt title: {plt_title}")
    for i in range(parallel_values.shape[0]):
        print("Board size: ", board_sizes_parallel[i])
        for j in range(parallel_values.shape[1]):
            print(nr_cores[i, j], parallel_values[i, j])
        print()
    print()

    # Plot diagonal
    plt_normal(parallel_values, nr_cores, board_sizes_parallel, plt_title, "diagonal.png")

    # Tridiagonal setup
    parallel_values_tri, schedulings, nr_cores, plt_title = read_parallel_times("triagonal_schedulings_result.txt")
    plt_normal(parallel_values_tri, nr_cores, schedulings, plt_title, "tridiagonal_schedulings.png")

    # SIMD INFLUECNE FULL_FULL
    parallel_values_full, simd_no_simd, nr_cores, plt_title = read_parallel_times("full_full_vs_simd.txt")
    plt_normal(parallel_values_full, nr_cores, simd_no_simd, plt_title, "full_full_vs_simd.png")

    # Different block sizes used
    parallel_values_blocks, blocks, nr_cores, plt_title = read_parallel_times("full_vs_blocked.txt")
    plt_normal(parallel_values_blocks, nr_cores, blocks, plt_title, "full_vs_blocked.png")

