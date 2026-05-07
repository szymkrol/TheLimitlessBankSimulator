| Name         | Symbol | Args                                               | Returns       |
|--------------|--------|----------------------------------------------------|---------------|
| Authenticate | 'A'    | `const char* login, const char* password`          | `int user_id` |
| GetBalance   | 'B'    | `int user_id`                                      | `int balance` |
| Deposit      | 'D'    | `int user_id, int amount`                          | `int success` |
| Withdraw     | 'W'    | `int user_id, int amount`                          | `int success` |
| Transfer     | 'T'    | `int user_id_source, int user_id_dest, int amount` | `int success` |
