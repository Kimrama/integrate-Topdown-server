FROM python:3.9-slim

WORKDIR /app

COPY downloads /app/downloads

EXPOSE 8000

CMD ["python3", "-m", "http.server", "8000", "--bind", "0.0.0.0"]
