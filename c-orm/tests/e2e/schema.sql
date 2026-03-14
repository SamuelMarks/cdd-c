-- E2E Complex Schema Test
CREATE TABLE users (
    id INT PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    age INT,
    score FLOAT,
    is_active BOOLEAN,
    created_at TIMESTAMP
);

CREATE TABLE posts (
    id INT PRIMARY KEY,
    user_id INT NOT NULL,
    title VARCHAR(255) NOT NULL,
    content TEXT,
    views BIGINT,
    published_date DATE,
    FOREIGN KEY (user_id) REFERENCES users(id)
);
