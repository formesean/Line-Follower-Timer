# Use Node.js image
FROM node:18

# Set working directory
WORKDIR /src/app

# Copy package.json and install dependencies
COPY package.json package-lock.json ./
RUN npm install

# Copy all source files
COPY . .

# Expose the Next.js port
EXPOSE 3000

# Build and start Next.js
RUN npm run build
CMD ["npm", "run", "dev"]
