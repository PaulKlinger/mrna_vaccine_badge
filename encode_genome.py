from typing import List, Dict
import sys


BASE_MAPPING = {"A": "00", "C": "01", "G": "10", "T": "11"}

def read_sequence(fasta_path: str) -> str:
    """ Read .fasta file and return only the sequence, removing comments and newlines
    """
    l = []
    with open(fasta_path, "r") as f:
        for line in f:
            if line.startswith(">"):
                continue
            l.append(line.replace("\n", ""))
    genome = "".join(l)
    return genome


def encode_sequence(genome: str) -> (List[int], int):
    """Encode sequence into bytes, using two bits per base according to BASE_MAPPING

    As the last byte may contain fewer than 4 bases the length of the sequence is
    returned separately.
    """

    num_bases = len(genome)
    sequence_uint8 = []

    i = 0
    while i < num_bases:
        byte_str = "".join(BASE_MAPPING[b] for b in genome[i:i+4])
        sequence_uint8.append(int(byte_str, 2))
        i += 4

    return sequence_uint8, num_bases


def decode(seq_ints: List[int], num_bases: int) -> str:
    """Decode sequence from bytes back to str of ACGT according to BASE_MAPPING"""

    int_to_base = {int(v, 2): k for k, v in BASE_MAPPING.items()}
    sequence = []
    i = 1
    for b in seq_ints:
        for s in [3, 2, 1, 0]:
            if i > num_bases:
                break
            sequence.append(int_to_base[(b >> (s * 2)) & 0b00000011])
            i += 1
    return "".join(sequence)


if __name__ == "__main__":
    genome = read_sequence(sys.argv[1])
    bytes_sequence, num_bases = encode_sequence(genome)

    # check that we get back the orginial sequence by decoding
    assert genome == decode(bytes_sequence, num_bases)
    # check that we generate valid uint8's
    assert all(0 <= b < 256 for b in bytes_sequence)

    for i in range(len(bytes_sequence) // 16 + 1):
        print(", ".join(str(b) for b in bytes_sequence[i * 16: (i + 1) * 16]) + ",")
    print()
    print(num_bases)


