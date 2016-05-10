import random
import string


def main():
    RECORD_SIZE = 1024
    number_of_records = 100

    with open('data'+str(number_of_records), 'w') as file:
        for i in range(0, number_of_records):
            record_id = str(i)+' '
            record_len = RECORD_SIZE - len(record_id)
            record = ''.join([random.choice(string.ascii_lowercase) for i in range(0, record_len)])
            file.write(record_id + record + '\n')


if __name__ == '__main__':
    main()